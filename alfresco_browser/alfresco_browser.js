// $Id$

Drupal.behaviors.attachBrowser = function(context) {
  $('div.alfresco-browser-container:not(.alfresco-browser-processed)', context).each(function() {
    $(this).append(Drupal.theme('alfrescoBrowserRemoveButton', this));
    var reference = $('input.alfresco-browser-reference', this);
    var name = $('input.alfresco-browser-name', this);
    var button = $('input.alfresco-browser-button', this);
    var remove = $('input.alfresco-browser-remove', this);
    button.click(function() {
      var popup = window.open(
        Drupal.settings.alfrescoBrowserUrl + '?r=' + reference.attr('id') + '&n=' + name.attr('id'),
        Drupal.settings.alfrescoBrowserName,
        Drupal.settings.alfrescoBrowserFeatures
      );
      if (popup) {
        popup.focus();
      }
    });
    remove.click(function() {
      reference.val('');
      name.val('');
      remove.hide();
    });
    reference.change(function() {
      if (reference.val() == '') {
        remove.hide();
      } else {
        remove.show();
      }
    });
    if (reference.val() == '') {
      // hide() no funciona en los fieldsets collapsibles
      remove.attr('style', 'display:none;');
    }
    $(this).addClass('alfresco-browser-processed');
  });
}

Drupal.theme.prototype.alfrescoBrowserRemoveButton = function(el) {
  return '&nbsp' + '<input type="button" class="alfresco-browser-remove" value="' + Drupal.t('Remove') + '" />';
}
