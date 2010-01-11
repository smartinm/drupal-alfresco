// $Id$

Drupal.behaviors.attachBrowser = function (context) {
  var winObjRef = null;
  $(document).ready(function() {
    $('#edit-alfresco-browser-button').click(function() {
      if (winObjRef == null || winObjRef.closed) {
        winObjRef = window.open(
          Drupal.settings.alfrescoBrowserUrl,
          Drupal.settings.alfrescoBrowserName,
          Drupal.settings.alfrescoBrowserFeatures);
      }
      winObjRef.focus();
      return false;
    });
  });
};
