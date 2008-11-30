<?php
// $Id$
 
/**
 * @file
 * Administrative page callbacks for the alfresco module. 
 */

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

/**
 * Form builder for the alfresco display settings page.
 *
 * @see system_settings_form()
 */
function alfresco_admin_display() {
  $form = array();
  
  $form['properties'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('Properties'),
    '#description' => t(''),
    '#collapsible' => FALSE,
    '#theme' => 'alfresco_properties_table'
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
 * @see http://cvs.drupal.org/viewvc.py/drupal/contributions/docs/developer/examples/scaffolding_example/scaffolding_example.admin.inc?revision=1.1
 * @see http://api.drupal.org/api/function/drupal_add_tabledrag/6
 */
function theme_alfresco_properties_table($form) {
  
  $rows = array();
  foreach(element_children($form) as $key) {
    $row = array();
    
    $row[] = drupal_render($form[$key]['key']);
    
    $form[$key]['weight']['#attributes']['class'] = 'imagecache-action-order-weight';     
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
  $output = theme('table', $header, $rows, array('id' => 'imagecache-preset-actions'));
  $output .= drupal_render($form);
  
  drupal_add_tabledrag('imagecache-preset-actions', 'order', 'self', 'imagecache-action-order-weight');
  
  return $output;
}
