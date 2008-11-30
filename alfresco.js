// $Id$

if (Drupal.jsEnabled) {
  $(document).ready(function() {
    $('input#edit-use-title').click(function() {
      if (this.checked) {
        $('#edit-title').attr('readonly', 'readonly');
      }
      else {
        $('#edit-title').removeAttr('readonly');
      }
    });
  });
}
