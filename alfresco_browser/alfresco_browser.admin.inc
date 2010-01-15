<?php
// $Id$

/**
 * @file
 * Administrative page callbacks for the alfresco browser module.
 */

// ------------------------------------------------------------------
// Alfresco browser settings page
// ------------------------------------------------------------------

/**
 * Form builder for the alfresco browser settings page.
 *
 * @see system_settings_form()
 */
function alfresco_browser_admin_settings() {

  $form['browser'] = array(
    '#type' => 'fieldset',
    '#title' => t('Browser'),
  );

  $form['browser']['alfresco_browser_home'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Home path'),
    '#description'   => t('Points to the root folder of Alfresco Repository (defaults to Company Home)'),
    '#required'      => TRUE,
    '#default_value' => variable_get('alfresco_browser_home', ALFRESCO_BROWSER_HOME),
  );

  $form['browser']['alfresco_browser_query_limit'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Page size'),
    '#description'   => t('The number of records to display per page (defaults to 20)'),
    '#required'      => TRUE,
    '#default_value' => variable_get('alfresco_browser_query_limit', 20),
  );

  $form['browser_cache'] = array(
    '#type' => 'fieldset',
    '#title' => t('Browser cache'),
  );

  $form['browser_cache']['alfresco_browser_cache_enabled'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable cache'),
    '#description'   => t('Enable cache for the Alfresco browser to store the results of web services queries against Alfresco repository.'),
    '#required'      => TRUE,
    '#default_value' => variable_get('alfresco_browser_cache_enabled', 1),
  );

  $form['#submit'][] = 'alfresco_browser_admin_settings_submit';
  return system_settings_form($form);
}

/**
 * Validate form values for alfresco browser settings page.
 */
function alfresco_browser_admin_settings_validate($form, &$form_state) {

  if (!alfresco_valid_path($form_state['values']['alfresco_browser_home'])) {
    form_set_error('alfresco_browser_home', t('The home path you specified is not valid.'));
  }
  else if (!alfresco_service_load_node($form_state['values']['alfresco_browser_home'])) {
    form_set_error('alfresco_browser_home', t('The home path you specified could not be found.'));
  }

  if (!is_numeric($form_state['values']['alfresco_browser_query_limit']) || $form_state['values']['alfresco_browser_query_limit'] < 1) {
    form_set_error('alfresco_browser_query_limit', t('Page size value must be a number and greater than zero.'));
  }
}

/**
 * Process admin alfresco browser settings form submissions.
 */
function alfresco_browser_admin_settings_submit($form, &$form_state) {

  if ($form_state['values']['alfresco_browser_cache_enabled'] != variable_get('alfresco_browser_cache_enabled', 1)) {
    cache_clear_all(NULL, 'cache_alfresco_browser');
    drupal_set_message(t('Alfresco browser cache cleared.'));
  }
}