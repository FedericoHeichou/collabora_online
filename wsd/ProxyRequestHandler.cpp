/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <Poco/URI.h>

#include <COOLWSD.hpp>
#include "ProxyRequestHandler.hpp"
#include <net/HttpRequest.hpp>
#include <net/HttpHelper.hpp>

std::map<std::string, std::shared_ptr<http::Response>> ProxyRequestHandler::CacheFileHash;
std::chrono::system_clock::time_point ProxyRequestHandler::MaxAge;

void ProxyRequestHandler::handleRequest(const std::string& relPath,
                                        const std::shared_ptr<StreamSocket>& socket)
{
    Poco::URI uriProxy(ProxyServer);
    const auto timeNow = std::chrono::system_clock::now();

    if (MaxAge > std::chrono::system_clock::time_point() && timeNow > MaxAge)
    {
        CacheFileHash.clear();
        MaxAge = std::chrono::system_clock::time_point();
    }

    const auto cacheEntry = CacheFileHash.find(relPath);
    if (cacheEntry != CacheFileHash.end())
    {
        socket->sendAndShutdown(*cacheEntry->second);
        return;
    }

    uriProxy.setPath(relPath);
    auto sessionProxy = http::Session::create(uriProxy.getHost(),
                                              http::Session::Protocol::HttpSsl,
                                              uriProxy.getPort());
    sessionProxy->setTimeout(std::chrono::seconds(10));
    http::Request requestProxy(uriProxy.getPathAndQuery());
    http::Session::FinishedCallback proxyCallback =
        [socket](const std::shared_ptr<http::Session>& httpSession)
            {
                try
                {
                    const auto callbackNow = std::chrono::system_clock::now();
                    std::shared_ptr<http::Response> httpResponse = httpSession->response();
                    if (httpResponse->statusLine().statusCode() == 200)
                    {
                        if (MaxAge == std::chrono::system_clock::time_point())
                        {
                            MaxAge = callbackNow + std::chrono::hours(10);
                        }

                        CacheFileHash[httpSession->getUrl()] = httpResponse;
                        socket->sendAndShutdown(*httpResponse);
                    }
                    else
                    {
                        HttpHelper::sendErrorAndShutdown(400, socket);
                    }
                }
                catch(...)
                {
                    LOG_DBG("ProxyCallback: Unknown exception");
                    HttpHelper::sendErrorAndShutdown(400, socket);
                }
            };

    sessionProxy->setFinishedHandler(proxyCallback);
    if (!sessionProxy->asyncRequest(requestProxy, *COOLWSD::getWebServerPoll()))
    {
        HttpHelper::sendErrorAndShutdown(400, socket);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */