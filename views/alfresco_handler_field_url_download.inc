<?php
// $Id$

/**
 * Field handler to provide simple renderer that turns a URL into a clickable link.
 *
 * @ingroup views_field_handlers
 */
class alfresco_handler_field_url_download extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
    $this->additional_fields['uid'] = array('table' => 'node', 'field' => 'uid');
    $this->additional_fields['type'] = array('table' => 'node', 'field' => 'type');
    $this->additional_fields['cm_name'] = 'cm_name';
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['display_as_link'] = array('default' => TRUE);

    return $options;
  }

  /**
   * Provide link to the page being visited.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['display_as_link'] = array(
      '#title' => t('Display as link'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['display_as_link']),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['display_as_link']) && user_access('download alfresco content')) {
      $node = new stdClass();
      $node->nid = $values->{$this->aliases['nid']};
      $node->uid = $values->{$this->aliases['uid']};
      $node->type = $values->{$this->aliases['type']};
      $node->cm_name = $values->{$this->aliases['cm_name']};
      $node->cm_content = TRUE; // @todo comprobar si es un fichero o no
      $url = alfresco_url_download($node);

      return l(check_plain($value), $url, array('html' => TRUE));
    }
    else {
      return $value;
    }
  }
}