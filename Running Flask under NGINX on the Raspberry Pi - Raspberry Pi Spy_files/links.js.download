jQuery(document).ready(function($) {
	EasyAzon_Addition_Localize_Links.process();
});

EasyAzon_Addition_Localize_Links.process = function() {
	var localizables = [];

	jQuery('a[data-localize="y"]').each(function(index, link) {
		var $link = jQuery(link);

		localizables.push($link.data());
	});

	if(localizables.length) {
		jQuery.post(
			EasyAzon_Addition_Localize_Links.ajaxUrl,
			{
				action:       EasyAzon_Addition_Localize_Links.ajaxAction,
				localizables: localizables
			},
			function(data, status) {
				var index, replacement;

				if(data && data.length) {
					for(index = 0; index < data.length; index++) {
						replacement = data[index];

						jQuery(replacement.selector).attr('href', replacement.url).attr('data-localized', 'y');
					}
				}
			},
			'json'
		);
	}
};