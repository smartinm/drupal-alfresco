// $Id$

Drupal.behaviors.attachBrowser = function(context) {
  $('div.alfresco-browser-container:not(.alfresco-browser-processed)', context).each(function() {
    var ref = $('input.alfresco-browser-reference', this).attr('id');
    var name = $('input.alfresco-browser-name', this).attr('id');
    $(this).addClass('alfresco-browser-processed').find(':button').click(function() {
      var popup = window.open(
        Drupal.settings.alfrescoBrowserUrl + '?r=' + ref + '&n=' + name,
        Drupal.settings.alfrescoBrowserName,
        Drupal.settings.alfrescoBrowserFeatures);
      if (popup) {
        popup.focus();
      }
    });
  });
}
