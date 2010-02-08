<?php
// $Id: alfresco_browser.pages.inc 1143 2010-02-02 10:39:51Z semm $

/**
 * @file
 *
 */

/**
 *
 */
function alfresco_browser_page() {
  global $language;

  $extjs_path  = module_exists('libraries') ? libraries_get_path('ext') : 'sites/all/libraries/ext';
  $icons_path  = drupal_get_path('module', 'alfresco') .'/images/filetypes';
  $module_path = drupal_get_path('module', 'alfresco_browser');

  alfresco_include('soap');
  $home_path = variable_get('alfresco_browser_home', ALFRESCO_BROWSER_HOME);
  $home_node = alfresco_soap_node_load($home_path);

  // Alfresco Browser Settings
  $settings = array('alfresco' => array(
    'homeRef'            => $home_node ? $home_node->id : '',
    'homeText'           => $home_node ? $home_node->cm_name : '- Error -',
    'iconsPath'          => url($icons_path),
    'serviceTreeUrl'     => url('alfresco/browser/service/spaces'),
    'serviceGridUrl'     => url('alfresco/browser/service/items'),
    'serviceDownloadUrl' => url('alfresco/browser/service/download'),
    'serviceUploadUrl'   => url('alfresco/browser/service/upload'),
    'serviceDeleteUrl'   => url('alfresco/browser/service/delete'),
    'queryLimit'         => variable_get('alfresco_browser_query_limit', 20),
    'accessAdd'          => user_access('add alfresco content'),
    'accessDelete'       => user_access('add alfresco content'),
  ));
  drupal_add_js($settings, 'setting');

  // CSS files
  drupal_add_css($extjs_path . '/resources/css/ext-all.css', 'module', 'all', FALSE);
  drupal_add_css($extjs_path . '/resources/css/xtheme-gray.css', 'module', 'all', FALSE);
  drupal_add_css($module_path . '/css/browser.css', 'module');

  // Ext JS files
  drupal_add_js($extjs_path .'/'. ALFRESCO_BROWSER_EXT_ADAPTER, 'module', 'footer', FALSE, TRUE, FALSE);
  drupal_add_js($extjs_path .'/'. ALFRESCO_BROWSER_EXT_ALL, 'module', 'footer', FALSE, TRUE, FALSE);

  // Ext JS locale files
  $locale_path = $extjs_path .'/source/locale/ext-lang-'. $language->language .'.js';
  if (!file_exists('./'. $locale_path)) {
    $locale_path = $extjs_path .'/src/locale/ext-lang-'. $language->language .'.js';
    if (!file_exists('./'. $locale_path)) {
      $locale_path = NULL;
    }
  }
  if ($locale_path) {
    drupal_add_js($locale_path, 'module', 'footer', FALSE, TRUE, FALSE);
  }

  // Alfresco Browser JS
  drupal_add_js($module_path .'/js/FileUploadField.js', 'module', 'footer', FALSE, TRUE, FALSE);
  drupal_add_js($module_path .'/js/AlfrescoBrowser.js', 'module', 'footer', FALSE, TRUE, FALSE);

  // Blank Image URL
  $blank_image_url = url($extjs_path .'/resources/images/default/s.gif');
  drupal_add_js("Ext.BLANK_IMAGE_URL = '$blank_image_url';", 'inline', 'footer');

  drupal_set_title(t('Alfresco Browser for Drupal'));
  echo theme('alfresco_browser_page');
}

/**
 *
 */
function alfresco_browser_repository_page() {
  drupal_add_css(drupal_get_path('module', 'alfresco_browser') .'/alfresco_browser.css');
  drupal_set_title('');
  $url = url('alfresco/browser');
  return '<iframe id="embedded-browser" frameborder="0"  src="'. $url .'"></iframe>';
}

function alfresco_browser_service_spaces() {

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
    $space = alfresco_soap_node_load($reference);

    if (!$space) {
      // node not found
      drupal_json(array());
      exit();
    }

    $nodes = array();

    $namespace = AlfNamespaceMap::getInstance();
    $folder_types = variable_get('alfresco_folder_types', array('cm_folder'));

    foreach ($space->getChildren() as $child) {
      /* @var $child_node AlfNode */
      $child_node = $child->child;

      // @todo: Enhanced suppport other folder types
      $type = $namespace->getShortName($child_node->getType());
      //$ctype = $session->getContentType(str_replace('_', ':', $type));

      if (in_array($type, $folder_types)) {
        $nodes[] = array(
          'id'   => $child_node->id,
          'text' => trim($child_node->cm_name),
          'qtip' => trim($child_node->cm_title),
        );
      }
    }

    usort($nodes, 'alfresco_browser_node_sort');

    if (variable_get('alfresco_browser_cache_enabled', TRUE)) {
      cache_set($cache_key, $nodes, 'cache_alfresco_browser', CACHE_TEMPORARY);
    }

    drupal_json($nodes);
  }
  catch (Exception $ex) {
  }
}


function alfresco_browser_service_items() {

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
    $session = alfresco_soap_get_session();

    $results = array();

    if (!empty($query)) {
      $store = new AlfStore($session, 'SpacesStore');
      $results = $session->query($store, '+ISNODE:T +TYPE:"cm:content" +@cm\:name:"*'. $query .'*"');
    }
    else {
      $reference = 'workspace://SpacesStore/'. $node;
      $folder = alfresco_soap_node_load($reference);

      if (!$folder) {
        drupal_json(array('total' => 0, 'rows' => array()));
        exit();
      }

      $namespace = AlfNamespaceMap::getInstance();
      $folder_types = variable_get('alfresco_folder_types', array('cm_folder'));

      foreach ($folder->children as $child_assoc) {
        // @todo: Enhanced suppport other folder types
        $type = $namespace->getShortName($child_assoc->child->type);
        if (!in_array($type, $folder_types)) {
          $results[] = $child_assoc->child;
        }
      }
    }

    if (count($results) == 0) {
      if (variable_get('alfresco_browser_cache_enabled', TRUE)) {
        cache_set($cache_key, array(), 'cache_alfresco_browser', CACHE_TEMPORARY);
      }

      drupal_json(array('total' => 0, 'rows' => array()));
      exit();
    }

    $module_path = drupal_get_path('module', 'alfresco_browser');
    $nodes = array();

    foreach ($results as $child_node) {
      $node_ref = $child_node->__toString();
      $nid = alfresco_exists_node($node_ref);
      $icon = alfresco_get_icon_filetype($child_node->cm_name, TRUE);

      if ($child_node->cm_content instanceof AlfContentData) {
        $content_data = $child_node->cm_content;
      }
      else {
        $content_data = NULL;
      }

      $nodes[] = array(
        'id'          => $child_node->id,
        'nid'         => $nid ? url('node/'. $nid, array('absolute' => TRUE)) : '',
        'name'        => trim($child_node->cm_name),
        'size'        => isset($content_data) ? format_size($content_data->size) : '',
        'mimetype'    => isset($content_data) ? $content_data->mimetype : '',
        'icon'        => $icon,
        'author'      => !is_null($child_node->cm_author) ? $child_node->cm_author : '',
        'creator'     => !is_null($child_node->cm_creator) ? $child_node->cm_creator : '',
        'modified'    => format_date(strtotime($child_node->cm_modified), 'custom', 'Y-m-d H:i:s'),
        'created'     => format_date(strtotime($child_node->cm_created), 'custom', 'Y-m-d H:i:s'),
        'description' => trim($child_node->cm_description),
        'title'       => trim($child_node->cm_title),
        'type'        => $namespace->getShortName($child_node->type),
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
 * Function used by usort to sort alfresco nodes by text.
 */
function alfresco_browser_node_sort($a, $b) {
  return strnatcmp($a['text'], $b['text']);
}
