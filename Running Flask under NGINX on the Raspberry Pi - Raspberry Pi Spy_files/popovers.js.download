jQuery(document).ready(function($) {
	var contents = {},
		current = null,
		loading = true,
		timeout = null;

	function getContent(element, container) {
		var $container = $(container),
			$element   = $(element),
			$data      = $element.data(),
			data       = {},
			identifier = $element.data('identifier'),
			locale     = $element.data('locale');

		if(contents[locale] && contents[locale][identifier]) {
			return contents[locale][identifier];
		} else {
			for(var p in $data) {
				if('object' !== typeof $data[p] && 'function' !== typeof $data[p]) {
					data[p] = $data[p];
				}
			}

			$.post(
				EasyAzon_Addition_Components_Popovers.ajaxUrl,
				{
					action: EasyAzon_Addition_Components_Popovers.ajaxAction,
					atts:   data,
				},
				function(data, status) {
					saveContent(identifier, locale, data.markup);

					$container.html(data.markup);
				},
				'json'
			);

			return saveContent(identifier, locale, EasyAzon_Addition_Components_Popovers.loading);
		}
	}

	function saveContent(identifier, locale, content) {
		if(!contents[locale]) {
			contents[locale] = {};
		}

		contents[locale][identifier] = content;

		return contents[locale][identifier];
	}

	function popoverTimeoutClear() {
		if(null !== timeout) {
			clearTimeout(timeout);
			timeout = null;
		}
	}

	function popoverTimeoutSet() {
		timeout = setTimeout(function() {
			if(null !== current) {
				$(current).removeClass('easyazon-popover-link-wrapper-hovered');

				current = null;
				timeout = null;
			}
		}, parseInt(EasyAzon_Addition_Components_Popovers.timeout));
	}

	$('[data-identifier][data-locale][data-popups="y"]').each(function(index, element) {
		var $element   = $(element).addClass('easyazon-popover-link'),
			$block     = $element.parents('.easyazon-block'),
			$container = $('<span class="easyazon-popover"></span>');

		if(!$block.size()) {
			$element.wrap('<span class="easyazon-popover-link-wrapper"></span>').parent().prepend($container);
			
			$element.parent().on('mouseenter', function(event) {
				var $wrapper = $(this),
					element  = $wrapper.get(0);

				popoverTimeoutClear();

				if(current !== element) {					
					if(null !== current) {
						$(current).removeClass('easyazon-popover-link-wrapper-hovered');
					}
					
					$container.html(getContent($element, $container));

					$wrapper.addClass('easyazon-popover-link-wrapper-hovered');

					current = element;
				}
			}).on('mouseleave', function(event) {
				popoverTimeoutSet();
			});
		}
	});

	$(document).on('mouseenter', '.easyazon-popover', function() {
		popoverTimeoutClear();
	}).on('mouseleave', '.easyazon-popover', function() {
		popoverTimeoutSet();
	});
});