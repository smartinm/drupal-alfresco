// $Id$

Drupal.behaviors.alfresco = function (context) {
  
  $('input#edit-use-title').click(function() {
    if (this.checked) {
      $('#edit-title').attr('readonly', 'readonly');
    }
    else {
      $('#edit-title').removeAttr('readonly');
    }
  });
}
