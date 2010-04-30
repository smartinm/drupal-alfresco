// $Id$

Drupal.behaviors.attachBrowser = function(context) {
  $('div.alfresco-browser-container:not(.alfresco-browser-processed)', context).each(function() {
    $(this).append(Drupal.theme('alfrescoBrowserRemoveButton', this)).addClass('alfresco-browser-processed');
    
    var inputRef = $(this).find('input.alfresco-browser-reference');
    var inputName = $(this).find('input.alfresco-browser-name');
    var btnBrowse = $(this).find('input.alfresco-browser-button');
    var btnRemove = $(this).find('input.alfresco-browser-remove');
    var divInfo = $(this).find('div.alfresco-browser-info');
    var attached = false;
    
    btnBrowse.click(function() {
      var popup = window.open(
        Drupal.settings.alfrescoBrowserUrl + '?r=' + inputRef.attr('id') + '&n=' + inputName.attr('id'),
        Drupal.settings.alfrescoBrowserName,
        Drupal.settings.alfrescoBrowserFeatures
      );
      if (popup) {
        popup.focus();
      }
      attached = true;
    });
    
    btnRemove.click(function() {
      if (attached || confirm(Drupal.t("Are you sure you want to remove '!name' from this content?", {'!name' : inputName.val()}))) {
        inputRef.val('');
        inputName.val('');
        btnRemove.hide();
        btnBrowse.removeAttr('disabled');
        divInfo.html('<em>' + Drupal.t('Changes will not be saved until the form is submitted.') + '</em>');
      }
    });
    
    inputRef.change(function() {
      if (!inputRef.val()) {
        btnRemove.hide();
        btnBrowse.removeAttr('disabled');
      } else {
        btnRemove.show();
        btnBrowse.attr('disabled', 'disabled');
      }
    });

    if (!inputRef.val()) {
      btnRemove.attr('style', 'display:none;');
    } else {
      btnBrowse.attr('disabled', 'disabled');
    }
  });
}

Drupal.theme.prototype.alfrescoBrowserRemoveButton = function () {
  return '&nbsp' + '<input type="button" class="alfresco-browser-remove" value="' + Drupal.t('Remove') + '" />';
};
