<?php
// $Id$
/**
 * @file
 *
 * Provide views data and handlers for alfresco module.
 * 
 * @see http://views-help.doc.logrus.com/help/views/api
 */

/**
 * Implementation of hook_views_data()
 */
function system_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // alfresco_node table

  $data['alfresco_node']['table']['group'] = t('Alfresco');
  $data['alfresco_node']['table']['join'] = array(
    'node' => array(
      // direct join to the node table via 'vid' field.
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      // direct join to the node_revisions table via 'vid' field.
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'users' => array(
      // direct join to the users table via 'uid' field.
      'left_field' => 'uid',
      'field' => 'uid',
     ),
  );

  // cm_name (filename)
  $data['alfresco_node']['cm_name'] = array(
    'title' => t('Name'),
    'help' => t('The name of the alfresco item.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  
  // timestamp field
  $data['alfresco_node']['timestamp'] = array(
    'title' => t('Upload date'),
    'help' => t('The date the file was uploaded.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  
  // ----------------------------------------------------------------------
  // alfresco_content table

  $data['alfresco_content']['table']['group'] = t('Alfresco');
  $data['alfresco_content']['table']['join'] = array(
    'node' => array(
      'left_table' => 'alfresco_node',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      'left_table' => 'alfresco_node',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );
  
  // url
  $data['alfresco_content']['url'] = array(
    'title' => t('URL'),
    'help' => t('The Guest URL of the alfresco file.'),
    'field' => array(
      'handler' => 'views_handler_field_url',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // mimetype
  $data['alfresco_content']['mimetype'] = array(
    'title' => t('Mime type'),
    'help' => t('The mime type of the alfresco file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // size
  $data['alfresco_content']['size'] = array(
    'title' => t('Size'),
    'help' => t('The size of the alfresco file.'),
    'field' => array(
      'handler' => 'views_handler_field_file_size',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function system_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/system',
    ),
    'handlers' => array(
      'views_handler_field_file' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_file_status' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_filter_file_status' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_argument_file_fid' => array(
        'parent' => 'views_handler_argument',
      ),
    ),
  );
}

function _views_file_status($choice = NULL) {
  $status = array(
    FILE_STATUS_TEMPORARY => t('Temporary'),
    FILE_STATUS_PERMANENT => t('Permanent'),
  );

  if (isset($choice)) {
    return isset($status[$choice]) ? $status[$choice] : t('Unknown');
  }

  return $status;
}


/**
 * @}
 */
