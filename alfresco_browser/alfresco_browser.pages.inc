<?php
// $Id$

/**
 * @file
 *
 */

/**
 *
 */
function alfresco_browser_repository_page() {
  drupal_set_title('');
  $url = url('alfresco/browser');
  $frame = '<iframe id="embedded-browser" style="width:100%;height:600px;border:2px solid #ccc;" frameborder="0"  src="'. $url .'"></iframe>';
  return $frame;
}

/**
 *
 */
function alfresco_browser_page() {
  $module_path = drupal_get_path('module', 'alfresco_browser');
  $extjs_path = module_exists('libraries') ? libraries_get_path('ext') : 'sites/all/libraries/ext';

  drupal_add_js($extjs_path .'/adapter/ext/ext-base.js', 'module', 'footer', FALSE, FALSE, FALSE);
  drupal_add_js($extjs_path .'/ext-all.js', 'module', 'footer', FALSE, FALSE, FALSE);
  drupal_add_js($extjs_path .'/source/locale/ext-lang-'. $language->language .'.js', 'module', 'footer', FALSE, FALSE, FALSE);

  drupal_add_js($module_path .'/browser/FileUploadField.js', 'module', 'footer');
  drupal_add_js($module_path .'/browser/browser.js', 'module', 'footer');

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
    alfresco_include('soap');

    $reference = 'workspace://SpacesStore/'. $node;
    $currentNode = alfresco_soap_node_load($reference);

    if (!$currentNode) {
      // node not found
      drupal_json(array());
      exit();
    }

    $nodes = array();
    $folder_types = variable_get('alfresco_folder_types', array('cm_folder'));
    $session = alfresco_soap_get_session();

    foreach ($currentNode->getChildren() as $child) {
      /* @var $childNode AlfNode */
      $childNode = $child->child;

      // @todo: Enhanced suppport other folder types
      $type = $session->getNamespaceMap()->getShortName($childNode->getType());

      //$ctype = $session->getContentType(str_replace('_', ':', $type));

      if (in_array($type, $folder_types)) {
        $nodes[] = array(
          'id'   => $childNode->id,
          'text' => $childNode->cm_name,
          'qtip' => $childNode->cm_title,
        );
      }
    }

    usort($nodes, '_alfresco_browser_node_sort');

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
        drupal_json(array('total' => count($cache->data), 'rows' => $rows));
        exit();
      }
    }
  }

  try {
    alfresco_include('soap');

    $queryResults = array();

    if (!empty($query)) {
      $spacesStore = new AlfStore($session, 'SpacesStore');
      $queryResults = $session->query($spacesStore, '+ISNODE:T +TYPE:"cm:content" +@cm\:name:"*'. $query .'*"');
    }
    else {
      $reference = 'workspace://SpacesStore/'. $node;
      $folder = alfresco_soap_node_load($reference);

      if (!$folder) {
        drupal_json(array('total' => 0, 'rows' => array()));
        exit();
      }

      $folder_types = variable_get('alfresco_folder_types', array('cm_folder'));
      $session = alfresco_soap_get_session();

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

      if ($childNode->cm_content instanceof AlfContentData) {
        $contentData = $childNode->cm_content;
      }

      $nodes[] = array(
        'id'          => $childNode->id,
        'nid'         => $nid ? url('node/'. $nid) : '',
        'name'        => $childNode->cm_name,
        'path'        => $childNode->getFolderPath(),
        'size'        => isset($contentData) ? format_size($contentData->size) : '',
        'mimetype'    => isset($contentData) ? $contentData->mimetype : '',
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

/**
 *
 */
function alfresco_browser_service_upload() {

  $space = isset($_POST['space']) ? trim($_POST['space']) : '';
  $name = isset($_POST['name']) ? trim($_POST['name']) : '';
  $title = isset($_POST['title']) ? trim($_POST['title']) : '';
  $description = isset($_POST['description']) ? trim($_POST['description']) : '';
  $author = isset($_POST['author']) ? trim($_POST['author']) : '';

  /*
  $validators = array(
    'file_validate_size' => array(file_upload_max_size())
  );
  */
  alfresco_include('soap');

  if ($file = file_save_upload('file')) {
    $filepath = $file->filepath;
    $mimetype = $file->filemime;
    // @see Fix for https://issues.alfresco.com/jira/browse/ETWOONE-400
    $encoding = 'UTF-8';
    $reference = 'workspace://SpacesStore/'. $space;

    $properties = array(
      'cm_title' => $title,
      'cm_description' => $description,
      'cm_author' => $author,
    );

    $result = alfresco_soap_node_add($reference, $filepath, $name, $mimetype, $encoding, 'cm_content', $properties);

    if (!isset($result->error)) {
      $success = array('success' => TRUE);
    }
    else {
      $success = array('success' => FALSE, 'error' => $result->error);
    }

    drupal_set_header('Content-Type: text/html;charset=UTF-8');
    echo drupal_to_js($success);
    exit();
  }
}

/**
 *
 */
function alfresco_browser_service_delete() {
  alfresco_include('soap');

  $node = isset($_GET['node']) ? trim($_GET['node']) : '';
  $reference = 'workspace://SpacesStore/'. $node;

  $result = alfresco_soap_node_delete($reference);
  dpm($result);
  if (isset($result->error)) {
    drupal_json(array('success' => FALSE, 'error' => $result->error));
    exit();
  }

  if ($nid = alfresco_exists_node($reference)) {
    node_delete($nid);
  }

  drupal_json(array('success' => TRUE));
}

/**
 * Función auxiliar para los nodos alfabéticamente por su nombre.
 */
function _alfresco_browser_node_sort($node1, $node2) {
  return strnatcmp($node1['text'], $node2['text']);
}
