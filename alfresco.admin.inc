<?php
// $Id$
 
/**
 * @file
 * 
 */

/**
 * 
 */
function alfresco_admin_overview(&$form_state) {

  $form['alfresco_repository_url'] = array(
    '#default_value'  => variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api'),
    '#description'    => t('The web service end point URL for the Alfresco repository. Example: http://localhost:8080/alfresco/api'),
    '#title'          => t('URL repository'),
    '#type'           => 'textfield',
  );
  $form['alfresco_credentials_username'] = array(
    '#default_value'  => variable_get('alfresco_credentials_username', 'guest'),
    '#description'    => t(''),
    '#title'          => t('Username'),
    '#type'           => 'textfield',
  );
  $form['alfresco_credentials_password'] = array(
    '#default_value'  => variable_get('alfresco_credentials_password', ''),
    '#description'    => t(''),
    '#title'          => t('Password'),
    '#type'           => 'password',
  );

  return system_settings_form($form); 
}
