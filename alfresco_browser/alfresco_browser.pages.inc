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

  $node = isset($_GET['node']) ? trim($_GET['node']) : '';
  $cache_clear = isset($_GET['cache']) ? trim($_GET['cache']) : '';

  if (empty($node)) {
    drupal_json(array());
    exit();
  }

  if (variable_get('alfresco_browser_cache_enabled', TRUE)) {
    $cache_key = 'tree='. $node;
    if ($cache_clear == 'all') {
      cache_clear_all('tree=', 'cache_alfresco_browser', TRUE);
    }
    else {
      $cache = cache_get($cache_key, 'cache_alfresco_browser');
      if ($cache && !empty($cache->data)) {
        $nodes = $cache->data;
        drupal_json($nodes);
        exit();
      }
    }
  }

  try {
    $reference = 'workspace://SpacesStore/'. $node;
    $session = alfresco_service_create_session();
    $currentNode = alfresco_service_load_node($reference, $session);

    if (!$currentNode) {
      // node not found
      drupal_json(array());
      exit();
    }

    $nodes = array();
    $folder_types = variable_get('alfresco_folder_types', array('cm_folder'));

    foreach ($currentNode->children as $child) {
      /* @var $childNode Node */
      $childNode = $child->child;

      // @todo: Enhanced suppport other folder types
      $type = $session->getNamespaceMap()->getShortName($childNode->getType());

      if (in_array($type, $folder_types)) {
        $nodes[] = array(
          'id'   => $childNode->id,
          'text' => $childNode->cm_name,
          'qtip' => $childNode->cm_title,
        );
      }
    }

    usort($nodes, '_alfresco_browser_sort_nodes');

    if (variable_get('alfresco_browser_cache_enabled', TRUE)) {
      cache_set($cache_key, $nodes, 'cache_alfresco_browser', CACHE_TEMPORARY);
    }

    drupal_json($nodes);
  }
  catch (Exception $ex) {
  }
}


function alfresco_browser_service_documents_json() {

  $node  = isset($_GET['node'])  ? trim($_GET['node']) : '';
  $query = isset($_GET['query']) ? trim($_GET['query']) : '';
  $start = isset($_GET['start']) ? intval($_GET['start']) : 0;
  $limit = isset($_GET['limit']) ? intval($_GET['limit']) : variable_get('alfresco_browser_query_limit', 20);
  $cache_clear = isset($_GET['cache']) ? trim($_GET['cache']) : '';

  if (empty($query) && empty($node)) {
    drupal_json(array('total' => 0, 'rows' => array()));
    exit();
  }

  if (variable_get('alfresco_browser_cache_enabled', TRUE)) {
    $cache_key = empty($query) ? 'node='. $node : 'query='. $query;
    switch ($cache_clear) {
    case 'node':
      cache_clear_all($cache_key, 'cache_alfresco_browser');
      break;

    case 'all':
      cache_clear_all('node=', 'cache_alfresco_browser', TRUE);
      cache_clear_all('query=', 'cache_alfresco_browser', TRUE);
      break;

    default:
      $cache = cache_get($cache_key, 'cache_alfresco_browser');
      if ($cache && !empty($cache->data)) {
        $rows = $limit > 0 ? array_slice($cache->data, $start, $limit) : $cache->data;
        drupal_json(array('total' => count($nodes), 'rows' => $rows));
        exit();
      }
    }
  }

  try {
    $session = alfresco_service_create_session();
    $queryResults = array();

    if (!empty($query)) {
      $spacesStore = new AlfStore($session, 'SpacesStore');
      $queryResults = $session->query($spacesStore, '+ISNODE:T +TYPE:"cm:content" +ALL:"*'. $query .'*"');
    }
    else {
      $reference = 'workspace://SpacesStore/'. $node;
      $folder = alfresco_service_load_node($reference, $session);

      if (!$folder) {
        drupal_json(array('total' => 0, 'rows' => array()));
        exit();
      }

      $folder_types = variable_get('alfresco_folder_types', array('cm_folder'));

      foreach ($folder->children as $childAssoc) {
        // @todo: Enhanced suppport other folder types
        $type = $session->getNamespaceMap()->getShortName($childAssoc->child->type);
        if (!in_array($type, $folder_types)) {
          $queryResults[] = $childAssoc->child;
        }
      }
    }

    if (count($queryResults) == 0) {
      if (variable_get('alfresco_browser_cache_enabled', TRUE)) {
        cache_set($cache_key, array(), 'cache_alfresco_browser', CACHE_TEMPORARY);
      }

      drupal_json(array('total' => 0, 'rows' => array()));
      exit();
    }

    $module_path = drupal_get_path('module', 'alfresco_browser');
    $nodes = array();

    foreach ($queryResults as $childNode) {
      $nodeRef = $childNode->__toString();
      $ext = substr(strrchr($childNode->cm_name, '.'), 1);
      $nid = alfresco_exists_node($nodeRef);

      $nodes[] = array(
        'id'          => $childNode->id,
        'nid'         => $nid ? url('node/'. $nid) : '',
        'name'        => $childNode->cm_name,
        'size'        => isset($childNode->cm_content) ? format_size($childNode->cm_content->size) : '',
        'mimetype'    => isset($childNode->cm_content) ? $childNode->cm_content->mimetype : '',
        'icon'        => file_exists($module_path .'/images/filetypes/'. $ext .'.gif') ? $ext : '_default',
        'author'      => $childNode->cm_author,
        'creator'     => $childNode->cm_creator,
        'modified'    => alfresco_date_convert($childNode->cm_modified),
        'created'     => alfresco_date_convert($childNode->cm_created),
        'description' => $childNode->cm_description,
        'title'       => $childNode->cm_title,
        'type'        => $childNode->type,
      );
    }

    if (variable_get('alfresco_browser_cache_enabled', TRUE)) {
      cache_set($cache_key, $nodes, 'cache_alfresco_browser', CACHE_TEMPORARY);
    }

    $rows = $limit > 0 ? array_slice($nodes, $start, $limit) : $nodes;
    drupal_json(array('total' => count($nodes), 'rows' => $rows));
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

  $mode = ($mode == 'inline' || $mode == 'attachment') ? $mode : NULL;
  $reference = 'workspace://SpacesStore/'. $node;

  alfresco_file_transfer($reference, $mode);
}

function _alfresco_browser_sort_nodes($node1, $node2) {
  return strnatcmp($node1['text'], $node2['text']);
}
