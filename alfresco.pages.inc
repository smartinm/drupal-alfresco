<?php
// $Id$

/**
 * @file
 * User page callbacks for the alfresco module.
 */

// ------------------------------------------------------------------
// Alfresco download page
// ------------------------------------------------------------------

/**
 * Menu callback; transfer alfresco file using the configured download method.
 */
function alfresco_page_download($node) {

  $node_link = l(t('view'), 'node/'. $node->nid);
  $watchdog_args = array('@type' => $node->type, '%title' => $node->title);
  watchdog('alfresco', '@type: downloaded %title.', $watchdog_args, WATCHDOG_NOTICE, $node_link);

  switch (variable_get('alfresco_file_downloads', ALFRESCO_FILE_DOWNLOADS_MODULE)) {
    case ALFRESCO_FILE_DOWNLOADS_MODULE:
      alfresco_file_transfer($node->reference);
      break;

    case ALFRESCO_FILE_DOWNLOADS_GUEST:
      if (isset($node->cm_content)) {
        drupal_goto($node->cm_content['url']);
      }
      break;

    case ALFRESCO_FILE_DOWNLOADS_TICKET:
      $alf_node = alfresco_service_load_node($node->reference);
      $alf_data = $alf_node->cm_content;
      if ($alf_data) {
        drupal_goto($alf_data->url);
      }
      break;
  }

  drupal_not_found();
  exit();
}

// ------------------------------------------------------------------
// Alfresco synchronize page
// ------------------------------------------------------------------

/**
 * Menu callback; synchronize Drupal Node data with Alfresco Node data.
 */
function alfresco_page_sync($node) {

  // Synchronize node
  alfresco_sync_node($node);

  $node_link = l(t('view'), 'node/'. $node->nid);
  $watchdog_args = array('@type' => $node->type, '%title' => $node->title);
  $t_args = array('@type'  => node_get_types('name', $node), '%title' => $node->title);

  watchdog('alfresco', '@type: synchronized %title.', $watchdog_args, WATCHDOG_NOTICE, $node_link);
  drupal_set_message(t('@type %title has been synchronized.', $t_args));

  // Redirect view node
  drupal_goto('node/'. $node->nid);
}


// ------------------------------------------------------------------
// Alfresco details page
// ------------------------------------------------------------------

/**
 * Menu callback; synchronize Drupal Node data with Alfresco Node data.
 */
function alfresco_page_details($node) {

  $alf_node = alfresco_service_load_node($node->reference);
  if (!$alf_node) {
    return $node;
  }

  $output = '';

  // Type
  $details = array();
  $details[] = array($alf_node->getType());
  $output .= theme('table', array(t('Type')), $details);

  // Aspects
  $details = array();
  foreach ($alf_node->getAspects() as $key => $value) {
    if (is_string($value) || is_null($value)) {
      $details[] = array($value);
    }
  }
  $output .= theme('table', array(t('Aspects')), $details);

  // Properties
  $details = array();
  foreach ($alf_node->getProperties() as $key => $value) {
    if (is_string($value) || is_null($value)) {
      $details[] = array($key, $value);
    }
  }
  $output .= theme('table', array(t('Properties'), t('Value')), $details);

  // VersionHistory
  $details = array();
  foreach ($alf_node->getVersionHistory() as $key => $value) {
    if (is_string($value) || is_null($value)) {
      $details[] = array($key, $value);
    }
  }
  $output .= theme('table', NULL, $details);

  // Associations
  $details = array();
  foreach ($alf_node->getAssociations() as $key => $value) {
    if (is_string($value) || is_null($value)) {
      $details[] = array($key, $value);
    }
  }
  $output .= theme('table', NULL, $details);

  return $output;
}

// ------------------------------------------------------------------
//
// ------------------------------------------------------------------

/**
 * Menu callback; Retrieve a JSON object containing autocomplete suggestions for existing alfresco nodes.
 */
function alfresco_autocomplete($string = '') {
  $matches = array();
  if ($string) {

    $results = alfresco_service_search($string);
    if (is_array($results)) {
      foreach ($results as $anode) {
        $matches[$anode->getId()] = check_plain($anode->getFolderPath()) .'/'. check_plain($anode->cm_name);
      }
    }
  }

  drupal_json($matches);
}

function alfresco_autocomplete2($string = '') {
  $matches = array();
  if ($string) {

    $results = alfresco_service_search($string);
    if (is_array($results)) {
      foreach ($results as $anode) {
        $matches[$anode->getId()] = check_plain($anode->getFolderPath()) .'/'. check_plain($anode->cm_name);
      }
    }
  }

  drupal_json($matches);
}


// ------------------------------------------------------------------
//
// ------------------------------------------------------------------

/**
 * @see https://issues.alfresco.com/jira/browse/ETWOONE-400
 * @see http://wiki.alfresco.com/wiki/PHP_Tutorial_Five
 * @see http://drupal.org/node/470834
 */
function alfresco_page_add($form_state) {

  // If a file has already been uploaded, we know we need the confirmation form.
  if (isset($form_state['values']['file'])) {
    return _alfresco_add_form_properties($form_state);
  }

  // But the default is the main form.
  return _alfresco_add_form($form_state);
}

function alfresco_page_add_submit($form, &$form_state) {
  dpm('alfresco_page_add_submit');

  $validators = array(
    'file_validate_extensions' => array(),
    'file_validate_image_resolution' => array(),
    'file_validate_size' => array(),
  );
  $validators = array();

  // Save new file uploads.
  if (user_access('upload alfresco files') && ($file = file_save_upload('file', $validators))) {
    $form_state['values']['file'] = (array)$file;
  }

  $form_state['rebuild'] = TRUE;
}

function _alfresco_add_form($form_state) {
  $form = array();
  $form['#attributes'] = array('enctype' => 'multipart/form-data');

  $form['upload'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload content'),
    '#description' => t('This dialog helps you to add content to a space.'),
  );

  $form['upload']['space'] = array(
    '#type' => 'textfield',
    '#title' => t('Space'),
    '#autocomplete_path' => 'alfresco/autocomplete',
    //'#description' => t('Space.'),
  );

  $form['upload']['file'] = array(
    '#type' => 'file',
    '#title' => t('File'),
    '#description' => t('Locate content to upload.'),
  );

  $form['buttons']['upload'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
  );

  return $form;
}

function _alfresco_add_form_properties($form_state) {

  $file = $form_state['values']['file'];
  if (empty($file)) {
    // TODO ?
  }
  //dpm($file);

  $form['#file'] = $file;

  drupal_set_message(t("'!filename' was uploaded successfully.", array(
    '!filename' => $file['filename'])
  ));

  $form['upload'] = array(
    '#type' => 'fieldset',
    '#title' => t('Uploaded content'),
  );

  $form['upload']['file'] = array(
    '#type' => 'item',
    '#value' => $file['filename'],
  );

  $form['properties'] = array(
    '#type' => 'fieldset',
    '#title' => t('General properties'),
  );

  $form['properties']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $file['filename'],
    '#required' => TRUE,
  );

  $form['properties']['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#default_value' => '{http://www.alfresco.org/model/content/1.0}content',
    '#options' => array(
      '{http://www.alfresco.org/model/content/1.0}content' => 'Content'
    ),
  );

  $encodings = array();
  foreach (mb_list_encodings() as $encoding) {
    $encodings[$encoding] = $encoding;
  }

  $form['properties']['encoding'] = array(
    '#type' => 'select',
    '#title' => t('Encoding'),
    '#default_value' => 'UTF-8',
    '#options' => $encodings,
  );

  $form['properties']['mimetype'] = array(
    '#type' => 'textfield',
    '#title' => t('Content Type'),
    '#default_value' => $file['filemime'],
  );

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('OK'),
    '#submit' => array('alfresco_page_add_upload'),
  );

  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('alfresco_page_add_cancel'),
  );

  return $form;
}

/**
 * TODO Controlar si el fichero ya existe.
 */
function alfresco_page_add_upload($form, &$form_state) {
  dpm('alfresco_page_add_upload');

  $file = $form['#file'];
  dpm($file);

  $session = alfresco_service_create_session();

  $guest_home = alfresco_service_load_node('/app:company_home/app:guest_home', $session);
  if (!$guest_home) {
    drupal_not_found();
    exit;
  }

  $filepath = $file['filepath'];

  $filename = $form_state['values']['name'];
  $mimetype = $form_state['values']['mimetype'];
  $encoding = $form_state['values']['encoding'];

  $contentUrl = alfresco_service_upload_file($filepath, $mimetype, $encoding, $session);
  if ($contentUrl) {
    $newNode = $guest_home->createChild('cm_content', 'cm_contains', $filename);

    $newNode->cm_name = $filename;
    //$newNode->cm_description = '';
    //$newNode->cm_title = '';
    $newNode->cm_content = $contentUrl;
  }
  $session->save();
}

function alfresco_page_add_cancel($form, &$form_state) {

}
