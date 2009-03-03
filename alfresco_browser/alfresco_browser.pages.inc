<?php
// $Id$

/**
 * @file
 *
 */

/**
 *
 */
function alfresco_browser_page() {
  print theme('alfresco_browser_page');
}

function alfresco_browser_json($action) {

  ////
  try {
    $nodeUUID = isset($_POST['node']) ? $_POST['node'] : NULL;

    $session = alfresco_service_create_session();
    $spacesStore = new Store($session, 'SpacesStore');

    if ($nodeUUID == NULL || $nodeUUID == 'NULL') {
      $results = $session->query($spacesStore, 'PATH:"'. ALFRESCO_BROWSER_HOME .'"');
      $currentNode = $results[0];
    }
    else {
      $nodeRef = 'workspace://SpacesStore/'. $nodeUUID;
      $currentNode = alfresco_service_load_node($nodeRef, $session);
    }

    if (!$currentNode) {
      // node not found
      drupal_json(array());
      exit();
    }

    if ($action == 'spaces') {
      $nodes = array();
      foreach ($currentNode->children as $child) {
        $childNode = $child->child;
        if ($childNode->type == ALFRESCO_TYPE_FOLDER) {
          $nodes[] = array(
            'id'   => $childNode->sys_node_uuid,
            'text' => $childNode->cm_name,
            'qtip' => $childNode->cm_title,
            //'leaf'    => count($childNode->children) == 0,
          );
        }
      }
      drupal_json($nodes);
      exit();
    }

    if ($action == 'items') {
      $items = array();
      $spaces = array();

      foreach ($currentNode->children as $child) {
        $childNode = $child->child;
        $item = array(
          'uuid'     => $childNode->sys_node_uuid,
          'name'     => $childNode->cm_name,
          'type'     => $childNode->type == ALFRESCO_TYPE_FOLDER ? 'space' : 'content',
          'size'     => $childNode->type == ALFRESCO_TYPE_CONTENT ? format_size($childNode->cm_content->size) : '',
          'created'  => alfresco_date_convert($childNode->cm_created),
          'modified' => alfresco_date_convert($childNode->cm_modified),
        );
        if ($childNode->type == ALFRESCO_TYPE_FOLDER) {
          $spaces[] = $item;
        }
        else {
          $items[] = $item;
        }
      }
      drupal_json(array('nodes' => array_merge($spaces, $items)));
      //drupal_json(array('nodes' => $items));
      exit();
    }
  }
  catch (Exception $ex) {
  }
}
