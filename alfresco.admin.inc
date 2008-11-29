<?php
// $Id$
 
/**
 * @file
 * 
 */

/**
 * 
 * @see alfresco_admin_form_submit
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

  $form['downloads'] = array(
    '#type' => 'fieldset',
    '#title' => t('Downloads'),
  );
  
  $form['downloads']['alfresco_file_downloads'] = array(
    '#type' => 'radios',
    '#title' => t('Download method'),
    '#default_value' => variable_get('alfresco_file_downloads', ALFRESCO_FILE_DOWNLOADS_DIRECT),
    '#options' => array(
      ALFRESCO_FILE_DOWNLOADS_DIRECT => t('Direct - files are available using Alfresco URL directly.'),
      ALFRESCO_FILE_DOWNLOADS_MODULE => t('Module - files are transferred by Drupal\'s alfresco module.'),
    ),
    '#description' => t('')
  ); 
  
  $form['#submit'][] = 'alfresco_admin_form_submit';

  return system_settings_form($form); 
}

/**
 * 
 */
function alfresco_admin_form_submit($form, &$form_state) {
  
  if (empty($form_state['values']['alfresco_credentials_password'])) {
    $password = variable_get('alfresco_credentials_password', 'guest');
    $form_state['values']['alfresco_credentials_password'] = $password;
  }
}
