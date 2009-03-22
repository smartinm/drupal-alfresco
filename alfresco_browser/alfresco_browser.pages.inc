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

function alfresco_browser_service_tree_json() {

  $node = isset($_GET['node']) ? trim($_GET['node']) : 'Home';
  $clear = isset($_GET['cache']) ? TRUE : FALSE;

  if (empty($node)) {
    drupal_json(array());
    exit();
  }

  if ($clear) {
    cache_clear_all('tree=', 'cache_alfresco_browser', TRUE);
  }

  $cache_key = 'tree='. $node;
  $cache = cache_get($cache_key, 'cache_alfresco_browser');
  if ($cache && !empty($cache->data)) {
    $nodes = $cache->data;
    drupal_json($nodes);
    exit();
  }

  try {
    $session = alfresco_service_create_session();

    if ($node == 'home') {
      $spacesStore = new Store($session, 'SpacesStore');
      $results = $session->query($spacesStore, 'PATH:"'. ALFRESCO_BROWSER_HOME .'"');
      $currentNode = $results[0];
    }
    else {
      $reference = 'workspace://SpacesStore/'. $node;
      $currentNode = alfresco_service_load_node($reference, $session);
    }

    if (!$currentNode) {
      // node not found
      drupal_json(array());
      exit();
    }

    $nodes = array();

    foreach ($currentNode->children as $child) {
      /* @var $childNode Node */
      $childNode = $child->child;
      if ($childNode->type == ALFRESCO_TYPE_FOLDER) {
        $nodes[] = array(
          'id'   => $childNode->id,
          'text' => $childNode->cm_name,
          'qtip' => $childNode->cm_title,
        );
      }
    }
    usort($nodes, create_function('$node_a, $node_b',
      'return strnatcmp($node_a[\'text\'], $node_b[\'text\']);')
    );
    drupal_json($nodes);
    cache_set($cache_key, $nodes, 'cache_alfresco_browser', CACHE_TEMPORARY);
  }
  catch (Exception $ex) {
    drupal_json(array());
  }
}


function alfresco_browser_service_documents_json() {

  $node  = isset($_GET['node'])  ? trim($_GET['node']) : '';
  $query = isset($_GET['query']) ? trim($_GET['query']) : '';
  $start = isset($_GET['start']) ? intval($_GET['start']) : 0;
  $limit = isset($_GET['limit']) ? intval($_GET['limit']) : variable_get('alfresco_browser_query_limit', 10);
  $clear = isset($_GET['cache']) ? TRUE : FALSE;

  if (empty($query) && empty($node)) {
    drupal_json(array('total' => 0, 'rows' => array()));
    exit();
  }

  $cache_key = empty($query) ? 'node='. $node : 'query='. $query;
  if ($clear) {
    cache_clear_all($cache_key, 'cache_alfresco_browser');
  }

  $cache = cache_get($cache_key, 'cache_alfresco_browser');
  if ($cache && !empty($cache->data)) {
    $nodes = $cache->data;
    drupal_json(array('total' => count($nodes), 'rows' => array_slice($nodes, $start, $limit)));
    exit();
  }

  try {
    $queryResults = array();
    $session = alfresco_service_create_session();

    if (!empty($query)) {
      $spacesStore = new Store($session, 'SpacesStore');
      $queryResults = $session->query($spacesStore, '+ISNODE:T +TYPE:"cm:content" +ALL:"*'. $query .'*"');
    }
    else {
      if ($node == 'home') {
        $spacesStore = new Store($session, 'SpacesStore');
        $results = $session->query($spacesStore, 'PATH:"'. ALFRESCO_BROWSER_HOME .'"');
        $folder = $results[0];
      }
      else {
        $reference = 'workspace://SpacesStore/'. $node;
        $folder = alfresco_service_load_node($reference, $session);
      }

      if (!$folder) {
        drupal_json(array('total' => 0, 'rows' => array()));
        exit();
      }

      foreach ($folder->children as $childAssoc) {
        if ($childAssoc->child->type != ALFRESCO_TYPE_FOLDER) {
          $queryResults[] = $childAssoc->child;
        }
      }
    }

    if (count($queryResults) == 0) {
      drupal_json(array('total' => 0, 'rows' => array()));
      cache_set($cache_key, array(), 'cache_alfresco_browser', CACHE_TEMPORARY);
      exit();
    }

    $nodes = array();
    global $base_path;

    foreach ($queryResults as $childNode) {
      $nodeRef = $childNode->__toString();
      $ext = substr(strrchr($childNode->cm_name, '.'), 1);
      $nid = alfresco_exists_node($nodeRef);

      $nodes[] = array(
        'id'          => $childNode->id,
        'nid'         => $nid ? url('node/'. $nid) : '',
        'name'        => $childNode->cm_name,
        'size'        => $childNode->type == ALFRESCO_TYPE_CONTENT ? format_size($childNode->cm_content->size) : '',
        'mimetype'    => $childNode->type == ALFRESCO_TYPE_CONTENT ? $childNode->cm_content->mimetype : '',
        'icon'        => file_exists(drupal_get_path('module', 'alfresco_browser') . '/images/filetypes/'. $ext .'.gif') ? $ext : '_default',
        'author'      => $childNode->cm_author,
        'creator'     => $childNode->cm_creator,
        'modified'    => alfresco_date_convert($childNode->cm_modified),
        'created'     => alfresco_date_convert($childNode->cm_created),
        'description' => $childNode->cm_description,
        'title'       => $childNode->cm_title,
        'type'        => $childNode->type,
      );
    }
    drupal_json(array('total' => count($nodes), 'rows' => array_slice($nodes, $start, $limit)));
    cache_set($cache_key, $nodes, 'cache_alfresco_browser', CACHE_TEMPORARY);
    exit();
  }
  catch (Exception $ex) {
  }
}

function alfresco_browser_service_download() {

  $node = isset($_GET['node']) ? trim($_GET['node']) : '';
  $mode = isset($_GET['mode']) ? trim($_GET['mode']) : '';

  if (empty($node)) {
    drupal_not_found();
    exit();
  }

  if ($mode != 'inline' && $mode != 'attachment') {
    $mode = NULL;
  }

  $reference = 'workspace://SpacesStore/'. $node;
  alfresco_file_transfer($reference, $mode);
}

/*
function alfresco_browser_json($action) {

  try {
    $nodeUUID = isset($_POST['node']) ? $_POST['node'] : NULL;

    $session = alfresco_service_create_session();
    $spacesStore = new Store($session, 'SpacesStore');

    if ($nodeUUID == NULL || $nodeUUID == 'null') {
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
      usort($nodes, '_alfresco_browser_order_nodes');
      drupal_json($nodes);
      exit();
    }

    if ($action == 'items') {
      $nodes = array();

      foreach ($currentNode->children as $child) {
        $childNode = $child->child;
        $nodeUUID = $childNode->sys_node_uuid;
        $nodeRef  = 'workspace://SpacesStore/'. $nodeUUID;

        $nodes[] = array(
          'uuid'     => $childNode->sys_node_uuid,
          'name'     => $childNode->cm_name,
          'title'    => $childNode->cm_title,
          'type'     => $childNode->type == ALFRESCO_TYPE_FOLDER ? '0' : '1',
          'size'     => $childNode->type == ALFRESCO_TYPE_CONTENT ? format_size($childNode->cm_content->size) : '',
          'created'  => alfresco_date_convert($childNode->cm_created),
          'modified' => alfresco_date_convert($childNode->cm_modified),
          'exists'   => alfresco_exists_node($nodeRef) ? '1' : '0',
        );
      }
      drupal_json(array('nodes' => $nodes));
      exit();
    }
  }
  catch (Exception $ex) {
  }
}

function _alfresco_browser_order_nodes($node1, $node2)
{
   return strcmp($node1['text'], $node2['text']);
}
*/
