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
function alfresco_node_download($node) {
  
  if (!user_access('download alfresco content')) {
    return drupal_access_denied();
  }
  
  switch (variable_get('alfresco_file_downloads', ALFRESCO_FILE_DOWNLOADS_MODULE)) {
    case ALFRESCO_FILE_DOWNLOADS_MODULE:
      alfresco_file_transfer($node);
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
function alfresco_node_sync($node) {
  
  $form_state = array();
  $form_state['values']['op'] = t('Save');
  module_load_include('inc', 'node', 'node.pages');
  drupal_execute('alfresco_item_node_form', $form_state, (object)$node);
  drupal_goto('node/'. $node->nid);
}
