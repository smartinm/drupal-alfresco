// $Id$

Drupal.behaviors.alfresco = function (context) {
}

Drupal.behaviors.alfrescoSettings = function (context) {
  var initialValue = '/DrupalWTF/';
  var submitButton = $('#edit-submit');
  var passwordInput = $('#edit-alfresco-credentials-password');
  if (!passwordInput.val()) {
    passwordInput.val(initialValue);
  }
  passwordInput.focus(function() {
    if($.trim(passwordInput.val()) == initialValue){
      passwordInput.val('');
    }
  });
  passwordInput.blur(function() {
    if($.trim(passwordInput.val()) == '') {
      passwordInput.val(initialValue);
    }
  });
  submitButton.click(function() {
    if ($.trim(passwordInput.val()) == initialValue) {
      passwordInput.val('');
    }
  });
}
