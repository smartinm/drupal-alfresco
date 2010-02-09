// $Id$

Drupal.behaviors.alfresco = function (context) {
}

Drupal.behaviors.alfrescoSettings = function (context) {
  var passwordInput = $('#edit-alfresco-credentials-password');
  if (!passwordInput.val()) {
    passwordInput.val('/DrupalWTF/');
  }
  $('#edit-submit').click(function() {
    if (passwordInput.val() == '/DrupalWTF/') {
      passwordInput.val('');
    }
  });
}
