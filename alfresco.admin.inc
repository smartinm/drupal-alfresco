<?php
// $Id$

/**
 * @file
 * Administrative page callbacks for the alfresco module.
 */

// ------------------------------------------------------------------
// Alfresco repository settings page
// ------------------------------------------------------------------

/**
 * Form builder for the alfresco settings page.
 *
 * @see alfresco_admin_overview_submit()
 */
function alfresco_admin_overview(&$form_state) {

  $form['repository'] = array(
    '#type' => 'fieldset',
    '#title' => t('Repository'),
  );

  $form['repository']['alfresco_repository_url'] = array(
    '#default_value' => variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api'),
    '#description'   => t('The web service end point URL for the Alfresco repository. Example: http://localhost:8080/alfresco/api'),
    '#title'         => t('URL repository'),
    '#type'          => 'textfield',
    '#required'      => TRUE,
  );

  $form['repository']['alfresco_credentials_username'] = array(
    '#default_value' => variable_get('alfresco_credentials_username', 'guest'),
    '#title'         => t('Username'),
    '#description'   => t(''),
    '#type'          => 'textfield',
    '#size'          => 25,
  );
  $form['repository']['alfresco_credentials_password'] = array(
    '#type'          => 'password_confirm',
    '#description'   => t('To change the current password, enter the new password in both fields. Default password is <em>guest</em>'),
    '#size'          => 25,
  );

  $download_help = t('Select the download method of the files stored in Alfresco repository:');
  $download_tips = array(
    t('<strong>Module method</strong>, use this method if you Alfresco repository is not publicly accessible (default).'),
    t('<strong>Direct guest method</strong>, this method use <em>URL Guest Access</em>, the <em>guest</em> Alfresco user must have access to the item referenced.'),
    t('<strong>Direct ticket method</strong>, this method use <em>Ticket URL Argument</em> to provide authentication without login.'),
  );
  $download_more = t('For more info, visit <a href="@url">URL Addressability page</a> on Alfresco Wiki.', array('@url' => 'http://wiki.alfresco.com/wiki/URL_Addressability#Security'));

  $form['downloads'] = array(
    '#type' => 'fieldset',
    '#title' => t('File downloads'),
    '#description' => $download_help . theme('item_list', $download_tips),
  );

  $form['downloads']['alfresco_file_downloads'] = array(
    '#type' => 'radios',
    '#title' => t('Download method'),
    '#default_value' => variable_get('alfresco_file_downloads', ALFRESCO_FILE_DOWNLOADS_MODULE),
    '#options' => array(
      ALFRESCO_FILE_DOWNLOADS_MODULE => t('Module - files are transferred by Drupal\'s alfresco module using <em>Ticket URL Argument</em> transparently.'),
      ALFRESCO_FILE_DOWNLOADS_GUEST  => t('Direct guest - files are available using Alfresco <em>URL Guest Access</em>.'),
      ALFRESCO_FILE_DOWNLOADS_TICKET => t('Direct ticket - files are available using Alfresco <em>Ticket URL Argument</em>.'),
    ),
    '#description' => $download_more,
  );

  $form['#submit'][] = 'alfresco_admin_overview_submit';

  return system_settings_form($form);
}

/**
 *
 */
function alfresco_admin_overview_submit($form, &$form_state) {

  if (empty($form_state['values']['alfresco_credentials_password'])) {
    $password = base64_decode(variable_get('alfresco_credentials_password', base64_encode('guest')));
  }
  else {
    $password = $form_state['values']['alfresco_credentials_password'];
  }

  $form_state['values']['alfresco_credentials_password'] = base64_encode($password);
}

// ------------------------------------------------------------------
// Alfresco display settings page
// ------------------------------------------------------------------

/**
 * Form builder for the alfresco display settings page.
 *
 * @see system_settings_form()
 */
function alfresco_admin_display() {
  $form = array();

  $form['properties'] = array(
    '#tree'        => TRUE,
    '#type'        => 'fieldset',
    '#title'       => t('Properties'),
    '#description' => t(''),
    '#collapsible' => FALSE,
    '#theme'       => 'alfresco_properties_table'
  );

  $mapping = array(
    'name'        => t('Name'),
    'title'       => t('Title'),
    'description' => t('Description'),
    'content'     => array(
      'mimetype'  => t('Mimetype'),
      'encoding'  => t('Encoding'),
      'size'      => t('Size'),
    ),
    'author'      => t('Author'),
    'creator'     => t('Creator'),
    'created'     => t('Created date'),
    'modifier'    => t('Modifier'),
    'modified'    => t('Modified date'),
  );

  $count = count($mapping);
  for ($delta = 0; $delta < $count; $delta++) {
    $property['key'] = array(
      '#type'  => 'markup',
      '#value' => 'cm:'. key($mapping),
    );

    $property['name'] = array(
      '#type'          => 'textfield',
      '#maxlength'     => '255',
      '#default_value' => current($mapping),
      '#disabled'      => TRUE,
      '#attributes'    => array('style' => 'width: 95%')
    );

    $property['display'] = array(
      '#type'          => 'checkbox',
      '#default_value' => TRUE,
    );

    $property['display'] = array(
      '#type'          => 'checkbox',
      '#default_value' => TRUE,
    );

    $property['title'] = array(
      '#type'          => 'radio',
      //'#name'          => 'title',
    );

    $property['body'] = array(
      '#type'          => 'radio',
      //'#name'          => 'body',
    );

    $property['show'] = array(
      '#type'          => 'select',
      '#default_value' => 2,
      '#options'       => array(0 => 'Only teaser', 1 => 'Only page', 2 => 'Both'),
      //'#attributes'    => array('style' => 'width: 95%')
    );

    $property['weight'] = array(
      '#type' => 'weight',
      '#default_value' => 0,
    );

    $form['properties'][$delta] = $property;

    next($mapping);
  }

  return system_settings_form($form);
}

/**
 *
 * @see http://cvs.drupal.org/viewvc.py/drupal/contributions/docs/developer/examples/scaffolding_example/scaffolding_example.admin.inc?revision=1.1
 * @see http://api.drupal.org/api/function/drupal_add_tabledrag/6
 */
function theme_alfresco_properties_table($form) {

  $rows = array();
  foreach (element_children($form) as $key) {
    $row = array();

    $row[] = drupal_render($form[$key]['key']);

    $form[$key]['weight']['#attributes']['class'] = 'alfresco-properties-order-weight';
    $row[] = drupal_render($form[$key]['name']);
    $row[] = drupal_render($form[$key]['display']);
    $row[] = drupal_render($form[$key]['title']);
    $row[] = drupal_render($form[$key]['body']);
    $row[] = drupal_render($form[$key]['show']);
    $row[] = drupal_render($form[$key]['weight']);

    $rows[] = array(
      'data'  => $row,
      'class' => 'draggable',
    );
  }

  $header = array(t('Property'), t('Name'), t('Display'), t('Title'), t('Body'),  t('Show'), '');
  $output = theme('table', $header, $rows, array('id' => 'alfresco-properties-table'));
  $output .= drupal_render($form);

  drupal_add_tabledrag('imagecache-preset-actions', 'order', 'self', 'alfresco-properties-order-weight');

  return $output;
}


// ------------------------------------------------------------------
// Alfresco advanced settings page
// ------------------------------------------------------------------

/**
 * Form builder for the alfresco advanced settings page.
 */
function alfresco_admin_advanced(&$form_state) {

  $form['alfresco_debug_output'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable debug output via the Devel module'),
    '#description' => t("Check this to enable Alfresco debug messages <em>if Devel is installed</em>."),
    '#default_value' => variable_get('alfresco_debug_output', ALFRESCO_DEBUG_OUTPUT),
  );

  $limits = drupal_map_assoc(array(10, 20, 50, 100, 200, 500));
  $period = drupal_map_assoc(array(900, 1800, 3600, 7200, 10800, 21600, 32400, 43200, 64800, 86400, 172800, 259200, 604800, 1209600, 2419200), 'format_interval');

  $form['synchronizing'] = array(
    '#type' => 'fieldset',
    '#title' => t('Synchronizing'),
  );

  $form['synchronizing']['alfresco_sync_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable node synchronize'),
    '#description' => t("Check this to enable Alfresco-Drupal node synchronizing. (<strong>Experimental</strong>)"),
    '#default_value' => variable_get('alfresco_sync_enabled', FALSE),
    '#disabled' => TRUE,  // @todo eliminar cuando sea estable
  );

  $form['synchronizing']['alfresco_sync_cron_limit'] = array(
    '#type' => 'select',
    '#title' => t('Number of items to synchronize per cron run'),
    '#default_value' => variable_get('alfresco_sync_cron_limit', 100),
    '#options' => $limits,
    '#description' => t('The maximum number of items synchronized in each pass of a <a href="@cron">cron maintenance task</a>. If necessary, reduce the number of items to prevent timeouts and memory errors while synchronizing.', array('@cron' => url('admin/reports/status'))),
    '#disabled' => TRUE,  // @todo eliminar cuando sea estable
  );

  $form['synchronizing']['alfresco_sync_cron_refresh'] = array(
    '#type' => 'select',
    '#title' => t('Synchronize interval'),
    '#default_value' => variable_get('alfresco_sync_cron_refresh', 21600),
    '#options' => $period,
    '#description' => t('The length of time between node synchronizes. (Requires a correctly configured <a href="@cron">cron maintenance task</a>)', array('@cron' => url('admin/reports/status'))),
    '#disabled' => TRUE,  // @todo eliminar cuando sea estable
  );

  return system_settings_form($form);
}
