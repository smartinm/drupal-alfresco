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

Drupal.behaviors.alfrescoAccountSettings = function (context) {

  $('#edit-alfresco-account').click(function(){
    if ($('#edit-alfresco-account').attr('checked')) {
      $('#edit-alfresco-username').attr('disabled','disabled');
      $('#edit-alfresco-password-pass1').attr('disabled','disabled');
      $('#edit-alfresco-password-pass2').attr('disabled','disabled');
    } else {
      $('#edit-alfresco-username').removeAttr('disabled');
      $('#edit-alfresco-password-pass1').removeAttr('disabled');
      $('#edit-alfresco-password-pass2').removeAttr('disabled');
    }
  });
  
  if ($('#edit-alfresco-account').attr('checked')) {
    $('#edit-alfresco-username').attr('disabled','disabled');
    $('#edit-alfresco-password-pass1').attr('disabled','disabled');
    $('#edit-alfresco-password-pass2').attr('disabled','disabled');
  }; 
}
