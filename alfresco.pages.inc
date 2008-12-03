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
  
  if (!user_access('download alfresco content')) {
    return drupal_access_denied();
  }
  
  $node_link = l(t('view'), 'node/'. $node->nid);
  $watchdog_args = array('@type' => $node->type, '%title' => $node->title);
  watchdog('alfresco', '@type: downloaded %title.', $watchdog_args, WATCHDOG_NOTICE, $node_link);
    
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
    if(is_string($value) || is_null($value)) {
      $details[] = array($value);
    }
  }
  $output .= theme('table', array(t('Aspects')), $details);
  
  // Properties
  $details = array();
  foreach ($alf_node->getProperties() as $key => $value) {
    if(is_string($value) || is_null($value)) {
      $details[] = array($key, $value);
    }
  }
  $output .= theme('table', array(t('Properties'), t('Value')), $details);

  // VersionHistory
  $details = array();
  foreach ($alf_node->getVersionHistory() as $key => $value) {
    if(is_string($value) || is_null($value)) {
      $details[] = array($key, $value);
    }
  }
  $output .= theme('table', NULL, $details);

  // Associations
  $details = array();
  foreach ($alf_node->getAssociations() as $key => $value) {
    if(is_string($value) || is_null($value)) {
      $details[] = array($key, $value);
    }
  }
  $output .= theme('table', NULL, $details);
  
  return $output;
}
