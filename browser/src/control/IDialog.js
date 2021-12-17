/* -*- js-indent-level: 8 -*- */
/*
 * L.IDialog
 */

L.IDialog = L.Class.extend({
	statics: {
		container: {},

		open: function (options) {
			var container, content;

			container = L.DomUtil.create('div', options.prefix + '-wrap');
			content = L.DomUtil.create('div', options.prefix + '-content', container);
			content.innerHTML = L.IDialog.innerHtml(options.message);
			content.appendChild(L.IDialog.createButtons(options.prefix, options.buttons));
			document.body.appendChild(container);
			L.IDialog.container = container;

			if (options.afterOpen && typeof options.afterOpen === 'function') {
				options.afterOpen.call(container);
			}
		},

		close: function () {
			if (L.IDialog.container)
				document.body.removeChild(L.IDialog.container);
		},

		innerHtml: function (string) {
			if (typeof string === 'undefined')
				return '';

			var elem = L.DomUtil.create('div', '');
			elem.appendChild(document.createTextNode(string));
			return elem.innerHTML;
		},

		createButtons: function (prefix, buttons) {
			var container, button, item, data;

			container = L.DomUtil.create('div', prefix + '-footer');
			for (item in buttons) {
				data = buttons[item];
				button = L.DomUtil.create('button', data.className, container);
				button.type = data.type;
				button.textContent = data.text;
			}
			return container;
		},

		isVisible: function () {
			return L.IDialog.container && L.IDialog.container.parentNode == document.body;
		}
	}
});
