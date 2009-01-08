<?php
// $Id$

/**
 * @file
 *
 */

/**
 *
 */
function alfresco_import_form() {
  $form = array();

  $form['reference'] = array(
    '#type' => 'textfield',
    '#title' => t('Space Reference'),
    '#required' => TRUE,
    '#description' => t('Alfresco Space Reference. Example: workspace://SpacesStore/uuid'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  return $form;
}

function alfresco_import_form_validate($form, &$form_state) {
  if (empty($form_state['values']['reference'])) {
    form_set_error('reference', t('You must set an Alfresco Space Reference.'));
  }
  $alf_node = alfresco_service_load_node($form_state['values']['reference']);
  if (empty($alf_node) /*|| $node->type != 'project_project' */) {
    form_set_error('reference', t('Invalid Alfresco Space Reference.'));
  }
}

function alfresco_import_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/content/alfresco_import/'. base64_encode($form_state['values']['reference']);
}

function alfresco_import_from_space($form, $reference) {
  dpm(base64_decode($reference));

  $form = array();

  if (!$reference) {
    drupal_set_message(t("You need to configure the import directory on the audio import module's <a href='!settings-page'>settings page</a>.", array('!settings-pages' => url('admin/settings/audio_import'))), 'error');
    return $form;
  }

  $form['#reference'] = $reference;
  $form['#node_type'] = 'alfresco_item';

  if (module_exists('taxonomy')) {
    // here's a little hack to get the taxonomy controls onto our form
    $form['type'] = array('#type' => 'value', '#value' => $form['#node_type']);
    $form['#node'] = new stdClass();
    $form['#node']->type = $form['#node_type'];
    taxonomy_form_alter($form, array(), $form['#node_type'] .'_node_form');
    unset($form['type']);
    unset($form['#node']);
  }

  $form['none_found'] = array(
    '#type' => 'item',
    '#value' => t('<em>No files were found.</em>'),
  );

  return $form;
}

function theme_alfresco_import_form($form) {
}
