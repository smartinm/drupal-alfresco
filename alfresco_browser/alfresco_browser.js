// $Id$

Drupal.behaviors.attachBrowser = function (context) {
  var popup = null;
  var $browseButton = $("#edit-alfresco-browser-button"); 
  
  $(document).ready(function() {
    $browseButton.click(function() {
      if (popup == null || popup.closed) {
        popup = window.open(
          Drupal.settings.alfrescoBrowserUrl,
          Drupal.settings.alfrescoBrowserName,
          Drupal.settings.alfrescoBrowserFeatures);
      }
      if (popup) {
        popup.focus();
      }
      return false;
    });
  });
};
