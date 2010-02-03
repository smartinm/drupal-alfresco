<?php
// $Id$

/**
 * @file
 * User page callbacks for the alfresco module.
 */

// ------------------------------------------------------------------
// Alfresco page
// ------------------------------------------------------------------

function alfresco_overview_page() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);
  return theme('admin_block_content', $content);
}

function alfresco_items_page() {
  //drupal_set_title(check_plain(''));
  $view = views_embed_view('alfresco_items');
  return $view;
}

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
      alfresco_include('soap');
      $alf_node = alfresco_soap_node_load($node->reference);
      $alf_data = $alf_node->cm_content;
      if ($alf_data) {
        drupal_goto($alf_data->url);
      }
      break;
  }

  drupal_not_found();
  exit();
}

function alfresco_download_content($workspace, $store, $uuid) {

  if (!alfresco_valid_uuid($uuid)) {
    drupal_not_found();
    exit();
  }

  $reference = "$workspace://$store/$uuid";
  alfresco_file_transfer($reference);
}

// ------------------------------------------------------------------
// Alfresco details page
// ------------------------------------------------------------------

/**
 * Menu callback; Alfresco node details.
 */
function alfresco_page_details($node) {

  alfresco_include('soap');
  $alf_node = alfresco_soap_node_load($node->reference);
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
    alfresco_include('soap');
    $results = alfresco_soap_search($string);
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
 * @todo: Arreglar flujo del formulario.
 *
 * @see https://issues.alfresco.com/jira/browse/ETWOONE-400
 * @see http://wiki.alfresco.com/wiki/PHP_Tutorial_Five
 * @see http://drupal.org/node/470834
 */
function alfresco_add_form($form_state) {

  // If a file has already been uploaded, we know we need the confirmation form.
  if (isset($form_state['values']['file'])) {
    return _alfresco_add_form_properties($form_state);
  }

  // But the default is the main form.
  return _alfresco_add_form($form_state);
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
    '#required' => TRUE,
    //'#autocomplete_path' => 'alfresco/autocomplete',
    '#default_value' => '/app:company_home/app:guest_home',
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

  $space = isset($form_state['values']['space']) ? $form_state['values']['space'] : '';

  // @todo $form_state['values']['file'] es vacío cuando se reconstruye el formularion
  $file = $form_state['values']['file'];
  $form['#file'] = $file;

  $form['upload'] = array(
    '#type' => 'fieldset',
    '#title' => t('Uploaded content'),
  );

  $form['upload']['space'] = array(
    '#type' => 'textfield',
    '#title' => t('Space'),
    '#required' => TRUE,
    //'#autocomplete_path' => 'alfresco/autocomplete',
    //'#description' => t('Space.'),
    '#default_value' => $space,
  );

  $form['upload']['file'] = array(
    '#type' => 'item',
    '#title' => t('File'),
    '#value' => $file['filename'] .' ('. format_size($file['filesize']) .')'
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
    '#maxlength' => 255,
  );

  $form['properties']['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#default_value' => 'cm_content',
    '#options' => array(
      'cm_content' => 'Content'
    ),
  );

  /*
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
  */

  $form['properties']['encoding'] = array(
    '#type' => 'textfield',
    '#title' => t('Encoding'),
    '#default_value' => 'UTF-8',
    '#maxlength' => 255,
  );

  $form['properties']['mimetype'] = array(
    '#type' => 'textfield',
    '#title' => t('Content Type'),
    '#default_value' => $file['filemime'],
    '#maxlength' => 255,
  );

  $form['content'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content properties'),
    '#description' => t('Information about the content to be uploaded.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['content']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => '',
    '#maxlength' => 255,
  );

  $form['content']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => '',
    '#rows' => 2,
  );

  $form['content']['author'] = array(
    '#type' => 'textfield',
    '#title' => t('Author'),
    '#default_value' => '',
    '#maxlength' => 255,
  );

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('OK'),
    '#submit' => array('alfresco_add_form_upload'),
  );

  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('alfresco_add_form_cancel'),
  );

  return $form;
}

function alfresco_add_form_validate($form, &$form_state) {

  // @todo: Añadir validadores
  if (empty($form['#file'])) {
    $validators = array(
      'file_validate_extensions' => array(),
      'file_validate_image_resolution' => array(),
      'file_validate_size' => array(),
    );
    $validators = array();

    // Save new file uploads.
    if ($file = file_save_upload('file', $validators)) {
      $form_state['values']['file'] = (array)$file;
    }
    else {
      form_set_error('file', t('File field is required.'));
    }
  }
}

function alfresco_add_form_submit($form, &$form_state) {
  //dpm('alfresco_page_add_submit');

  // Save new file uploads.
  if (!empty($form_state['values']['file'])) {
    /*
    drupal_set_message(t("'!filename' was uploaded successfully.", array(
      '!filename' => $form_state['values']['file']['filename'])
    ));
    */
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * @todo: Controlar si el fichero ya existe.
 */
function alfresco_add_form_upload($form, &$form_state) {
  //dpm('alfresco_page_add_upload');

  $file = $form['#file'];
  $space = $form_state['values']['space'];

  if ($file) {
    $filepath = $file['filepath'];
    $name = $form_state['values']['name'];
    $type = $form_state['values']['type'];
    $mimetype = $form_state['values']['mimetype'];
    $encoding = $form_state['values']['encoding'];

    $properties = array(
      'cm_title' => $form_state['values']['title'],
      'cm_description' => $form_state['values']['description'],
      'cm_author' => $form_state['values']['author'],
    );

    alfresco_include('soap');
    $result = alfresco_soap_node_add($space, $filepath, $name, $mimetype, $encoding, $type, $properties);
    if (!isset($result->error)) {
      $name = $result->node->cm_name;
      drupal_set_message(t("'!name' was created successfully.", array('!name' => $name)));
    }
    else {
      form_set_error('file', $result->error);

      $form_state['values']['file'] = (array)$file;
      $form_state['rebuild'] = TRUE;
    }
  }
}

function alfresco_add_form_cancel($form, &$form_state) {
  //dpm('alfresco_page_add_cancel');
  //drupal_goto('alfresco/add');
}
