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
    '#description'   => '',
    '#type'          => 'textfield',
    '#size'          => 25,
  );
  $form['repository']['alfresco_credentials_password'] = array(
    '#type'          => 'password_confirm',
    '#description'   => t('To change the current password, enter the new password in both fields. Default password is <em>guest</em>'),
    '#size'          => 25,
  );

  $auth_help = t('Select the authentication method to connect Alfresco repository.');
  $auth_more = t('Anonymous users always uses account repository.');

  $form['authentication'] = array(
    '#type' => 'fieldset',
    '#title' => t('Authentication'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => $auth_help,
  );

  $form['authentication']['alfresco_authentication_method'] = array(
    '#type' => 'radios',
    '#title' => t('Choose authentication mode'),
    '#default_value' => variable_get('alfresco_authentication_method', ALFRESCO_AUTH_SINGLE),
    '#required' => TRUE,
    '#options' => array(
      ALFRESCO_AUTH_SINGLE   => t('Single. Use the above username and password to connect to repository.'),
      ALFRESCO_AUTH_PER_USER => t('Per user. Use users accounts and passwords to connect to repository.'),
    ),
    '#description' => $auth_more,
  );

  $form['authentication']['alfresco_authentication_admin'] = array(
    '#type' => 'checkbox',
    '#title' => t('Admin use account repository'),
    '#default_value' => variable_get('alfresco_authentication_admin', TRUE),
    '#required' => TRUE,
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
    '#title' => t('Download method'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => $download_help . theme('item_list', $download_tips),
  );

  $form['downloads']['alfresco_file_downloads'] = array(
    '#type' => 'radios',
    '#title' => t('Choose download method'),
    '#default_value' => variable_get('alfresco_file_downloads', ALFRESCO_FILE_DOWNLOADS_MODULE),
    '#options' => array(
      ALFRESCO_FILE_DOWNLOADS_MODULE => t('Module - files are transferred by Drupal\'s alfresco module using <em>Ticket URL Argument</em> transparently (recommended).'),
      ALFRESCO_FILE_DOWNLOADS_GUEST  => t('Direct guest - files are available using Alfresco <em>URL Guest Access</em>.'),
      ALFRESCO_FILE_DOWNLOADS_TICKET => t('Direct ticket - files are available using Alfresco <em>Ticket URL Argument</em>.'),
    ),
    '#description' => $download_more,
  );

  $form['#validate'][] = 'alfresco_admin_overview_validate';
  $form['#submit'][] = 'alfresco_admin_overview_submit';

  return system_settings_form($form);
}

/**
 * Validate the alfresco overview settings form.
 */
function alfresco_admin_overview_validate($form, &$form_state) {
  // Ensure URL is valid.
  if (!valid_url($form_state['values']['alfresco_repository_url'], TRUE)) {
    form_set_error('url', t('The URL repository %url is invalid. Please enter a fully-qualified URL, such as http://share.alfresco.com/alfresco/api.', array('%url' => $form_state['values']['alfresco_repository_url'])));
  }
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

  //$link = array('!link' => l(t('the status report'), 'admin/reports/status'));
  //drupal_set_message(t("Check !link to see whether the connection was successful.", $link));

  variable_set('alfresco_http_request_fails', FALSE);
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
    '#description' => t('Sorry, this settings not working (under development).'),
    '#collapsible' => FALSE,
    '#theme'       => 'alfresco_properties_table'
  );

  $mapping = array(
    'name'        => t('Name'),
    'title'       => t('Title'),
    'description' => t('Description'),
    /*
    'content'     => array(
      'mimetype'  => t('Mimetype'),
      'encoding'  => t('Encoding'),
      'size'      => t('Size'),
    ),
    */
    'author'      => t('Author'),
    'creator'     => t('Creator'),
    'created'     => t('Created date'),
    'modifier'    => t('Modifier'),
    'modified'    => t('Modified date'),
  );

  $count = count($mapping);
  for ($delta = 0; $delta < $count; $delta++) {
    $key = key($mapping);

    $property['key'] = array(
      '#type'  => 'markup',
      '#value' => 'cm:'. $key,
    );

    $property['name'] = array(
      '#type'          => 'textfield',
      '#maxlength'     => '255',
      '#default_value' => current($mapping),
      '#disabled'      => TRUE,
      //'#attributes'    => array('style' => 'width: 95%')
    );

    $property['display'] = array(
      '#type'          => 'checkbox',
      '#default_value' => TRUE,
    );

    $property['title'] = array(
      '#type'          => 'radio',
      '#name'          => 'title',
      '#default_value' => NULL,
      '#return_value'  => $key,
    );

    $property['body'] = array(
      '#type'          => 'radio',
      '#name'          => 'body',
      '#default_value' => NULL,
      '#return_value'  => $key,
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
    $row[] = drupal_render($form[$key]['name']);
    $row[] = drupal_render($form[$key]['display']);
    $row[] = drupal_render($form[$key]['title']);
    $row[] = drupal_render($form[$key]['body']);
    $row[] = drupal_render($form[$key]['show']);

    $form[$key]['weight']['#attributes']['class'] = 'alfresco-properties-order-weight';
    $row[] = drupal_render($form[$key]['weight']);

    $rows[] = array(
      'data'  => $row,
      'class' => 'draggable',
    );
  }

  $header = array(t('Property'), t('Name'), t('Display'), t('Title'), t('Body'),  t('Show'), '');
  $output = theme('table', $header, $rows, array('id' => 'alfresco-properties-table'));
  $output .= drupal_render($form);

  drupal_add_tabledrag('alfresco-properties-table', 'order', 'self', 'alfresco-properties-order-weight');

  return $output;
}


// ------------------------------------------------------------------
// Alfresco advanced settings page
// ------------------------------------------------------------------

/**
 * Form builder for the alfresco advanced settings page.
 */
function alfresco_admin_advanced(&$form_state) {

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

  $tip = t('El WSDL del DictionaryService que incluye Alfresco tiene un error en la definicion del namespace que provoca un error al importarlo, se ha includo una versión corregida el WSDL con el módulo. Marque esta casilla para utilizar el fichero dictionary-service.wsdl.');
  $tipo_more = t('For more info, visit <a href="@url">ALFCOM-1913 issue</a> on Alfresco JIRA.', array(
    '@url' => 'https://issues.alfresco.com/jira/browse/ALFCOM-1913')
  );

  $form['alfresco_workaround_dictionary_service_wsdl'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use workaround for incorrect targetNamespace WSDL.'),
    '#description' => $tip . $tipo_more,
    '#default_value' => variable_get('alfresco_local_dictionary_service_wsdl', TRUE),
  );

  return system_settings_form($form);
}
