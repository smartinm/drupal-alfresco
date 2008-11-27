<?php
// $Id$

/**
 * @file
 * Implementación de las operaciones CRUD sobre el tipo de contenido.
 */

function _alfresco_exception_handler($excepcion) {
  drupal_set_message($excepcion->getMessage(), 'error');
}

/**
 * Implementation of hook_form().
 *
 * Now it's time to describe the form for collecting the information
 * specific to this node type. This hook requires us to return an array with
 * a sub array containing information for each element in the form.
 */
function alfresco_form(&$node, $form_state) {

  drupal_add_js(drupal_get_path('module', 'alfresco') .'/alfresco.js');
  
  // The site admin can decide if this node type has a title and body, and how
  // the fields should be labeled. We need to load these settings so we can
  // build the node form correctly.
  $type = node_get_types('type', $node);

  $form['reference'] = array(
    '#type' => 'textfield',
    '#title' => t('Node Reference'),
    '#required' => TRUE,
    '#default_value' => $node->reference,
    '#weight' => -6,
    '#description' => t('Alfresco Node Reference. Example: workspace://SpacesStore/uuid'),
  );
  
  if ($type->has_title) {
    $form['use_title'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use Alfresco node title.'),
      '#description' => t('Deje esta campo marcado para obtener el título automáticamente del nodo de Alfresco, si desea indicar un título desmárquelo.'),
      '#default_value' => TRUE,
      '#weight' => -5
    );
    
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => check_plain($type->title_label),
      '#required' => FALSE,
      '#default_value' => $node->title,
      '#weight' => -5,
      '#disabled' => TRUE,
    );
  }

  if ($type->has_body) {
    // In Drupal 6, we can use node_body_field() to get the body and filter
    // elements. This replaces the old textarea + filter_form() method of
    // setting this up. It will also ensure the teaser splitter gets set up
    // properly.
    $form['body_field'] = node_body_field($node, $type->body_label, $type->min_word_count);
  }
  
  $form['#validate'][] = 'alfresco_form_validate';
  $form['#submit'][] = 'alfresco_form_submit';

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function alfresco_validate($node, &$form) {
  dpm("alfresco_validate");
}

/**
 * Implementation of hook_validate().
 * 
 * Valida que la referencia al nodo de alfresco es correcta, comprueba que el nodo existe y obtiene el título. 
 * 
 * @see alfresco_form_submit
 * @see http://drupal.org/node/144132#passing
 */
function alfresco_form_validate($form, &$form_state) {
  dpm("alfresco_form_validate");
  
  if ($form_state['values']['reference']) {
    $nodeRef = $form_state['values']['reference'];
    
    if (!alfresco_valid_reference($nodeRef)) {
      form_set_error('reference', t('La referencia al nodo de Alfresco no tiene el formato correcto.'));
    }
    else {
      $alf_node = alfresco_service_get_node($nodeRef);
      
      if (is_null($alf_node)) {
        form_set_error('reference', t('La referencia al nodo de Alfresco no existe.'));
      }
      else {
        $form_state['alf_node'] = $alf_node;
      }
    }
  }
}

/**
 *
 */
function alfresco_form_submit($form, &$form_state) {
  dpm("alfresco_form_submit");

  if (isset($form_state['alf_node'])) {
    $alf_node = $form_state['alf_node'];
    
    if (!empty($form_state['values']['use_title'])) {
      $form_state['values']['title'] = $alf_node->cm_title;
    }
  }
}

/**
 * Implementation of hook_insert().
 */
function alfresco_insert($node) {
  db_query("INSERT INTO {alfresco} (vid, nid, reference) VALUES (%d, %d, '%s')", $node->vid, $node->nid, $node->reference);
}

/**
 * Implementation of hook_update().
 *
 * As an existing node is being updated in the database, we need to do our own
 * database updates.
 */
function alfresco_update($node) {

  // if this is a new node or we're adding a new revision,
  if ($node->revision) {
    alfresco_insert($node);
  }
  else {
    db_query("UPDATE {alfresco} SET reference = '%s' WHERE vid = %d", $node->reference, $node->vid);
  }
}

/**
 * Implementation of hook_nodeapi().
 *
 * When a node revision is deleted, we need to remove the corresponding record
 * from our table. The only way to handle revision deletion is by implementing
 * hook_nodeapi().
 */
function alfresco_nodeapi(&$node, $op, $a3 = NULL, $a4 = NULL) {
  
  switch ($op) {
    case 'delete revision':
      // Notice that we're matching a single revision based on the node's vid.
      db_query('DELETE FROM {alfresco} WHERE vid = %d', $node->vid);
      break;
  }
}

/**
 * Implementation of hook_delete().
 *
 * When a node is deleted, we need to remove all related records from out table.
 */
function alfresco_delete($node) {
  
  // Notice that we're matching all revision, by using the node's nid.
  db_query('DELETE FROM {alfresco} WHERE nid = %d', $node->nid);
}

/**
 * Implementation of hook_load().
 *
 * Now that we've defined how to manage the node data in the database, we
 * need to tell Drupal how to get the node back out. This hook is called
 * every time a node is loaded, and allows us to do some loading of our own.
 */
function alfresco_load($node) {
  
  $additions = db_fetch_object(db_query('SELECT reference FROM {alfresco} WHERE vid = %d', $node->vid));
  return $additions;
}

/**
 * Implementation of hook_view().
 *
 * This is a typical implementation that simply runs the node text through
 * the output filters.
 */
function alfresco_view($node, $teaser = FALSE, $page = FALSE) {
  $node = node_prepare($node, $teaser);
  //return $node;

  try {
    // Include the required Alfresco PHP API objects  
    require_once "Alfresco/Service/Repository.php";
    require_once "Alfresco/Service/Session.php";
    require_once "Alfresco/Service/SpacesStore.php";
  
    // The web service end point URL for the Alfresco repository
    $repositoryUrl = variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api');
    
    // The user credentials we are going to use
    $userName = variable_get('alfresco_credentials_username', 'guest');
    $password = variable_get('alfresco_credentials_password', 'guest'); 
    
    // Create the repository object baseed on the repository URL.  The repository represents the Alfresco repository we are connecting to.
    // It can be used to authenticate users and as a session factory.
    $repository = new Repository($repositoryUrl);
    
    // Authenticate the user details for the repository we want to connect to.  Assuming authentication is succesful we will be returned
    // a ticket.  The ticket is a string which can be used to create Session's
    $ticket = $repository->authenticate($userName, $password);
    
    // Using the ticket we can now create a session.  In general it is better to create a new session for a new page or page sequence rather 
    // than creating one and using across many pages as this helps to prevent resource usage from creaping up.
    $session = $repository->createSession($ticket);
    //$session->getStoreFromString()
  
    // Create a reference to the 'SpacesStore'
    $spacesStore = new SpacesStore($session);
    
    $currentNode = $session->getNode($spacesStore, '38985dc2-bc5e-11dd-8590-695888b0c20c');
    
    $properties = array();
    foreach ($currentNode->properties as $name => $value) {
      if(is_string($value)) {
        $properties[] = array($name, $value);
      }
      else {
        $properties[] = array($name, $value->mimetype);
      }
    }
    
    $contentData = $currentNode->cm_content;
    
    $node->content['alfresco'] = array(
      '#value' => theme('table', array('Propiedad', 'Valor'), $properties) . l('Descarga', $contentData->getUrl()),
      '#weight' => 1,
    );
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    return $node;
  }
  
  /*
  if (ini_get('allow_url_fopen') == '1') {
    dpm("allow_url_fopen");
    $url = $contentData->getUrl(); 
    // make sure the remote file is successfully opened before doing anything else
    if ($fp = fopen($url, 'r')) {
      dpm("fopen");
       $content = '';
       // keep reading until there's nothing left
       while ($line = fread($fp, 1024)) {
          $content .= $line;
       }
    
       print $content;
    } else {
      dpm("ERROR");
       // an error occured when trying to open the specified url
    }
  } 
  */

  return $node;
}


//////////

/**
 * @see http://wiki.alfresco.com/wiki/Web_Service_Data_Types
 */
function alfresco_valid_reference($reference) {
  $scheme = '\w+';
  $address = '\w+';
  $uuid = '[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}';
  return preg_match("/^$scheme:\/\/$address\/$uuid$/", $reference);
}

/**
 * 
 */
function alfresco_parse_reference($reference) {
  
  $info = array();

  $scheme = '\w+';
  $address = '\w+';
  $uuid = '[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}';
  
  if (preg_match("/^($scheme):\/\/($address)\/($uuid)$/", $reference, $matches)) {
    $info['scheme'] = $matches[1];
    $info['address'] = $matches[2];
    $info['uuid'] = $matches[3];
  }
  return $info;
}


function alfresco_service_include_api() {
  static $included = FALSE;
  
  // Ensure this only gets run once.
  if ($included) {
    return;
  }
  
  // Include the required Alfresco PHP API objects  
  require_once "Alfresco/Service/Repository.php";
  require_once "Alfresco/Service/Session.php";
  //require_once "Alfresco/Service/SpacesStore.php";
  
  $included = TRUE;
}

function alfresco_service_create_session() {

  try {
    //alfresco_service_include_api();
    
    // Include the required Alfresco PHP API objects  
    require_once "Alfresco/Service/Repository.php";
    require_once "Alfresco/Service/Session.php";
    
    // The web service end point URL for the Alfresco repository
    $repositoryUrl = variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api');
    
    // The user credentials we are going to use
    $userName = variable_get('alfresco_credentials_username', 'guest');
    $password = variable_get('alfresco_credentials_password', 'guest'); 
    
    // Create the repository object baseed on the repository URL.  The repository represents the Alfresco repository we are connecting to.
    // It can be used to authenticate users and as a session factory.
    $repository = new Repository($repositoryUrl);
    
    // Authenticate the user details for the repository we want to connect to.  Assuming authentication is succesful we will be returned
    // a ticket.  The ticket is a string which can be used to create Session's
    $ticket = $repository->authenticate($userName, $password);
    
    // Using the ticket we can now create a session.  In general it is better to create a new session for a new page or page sequence rather 
    // than creating one and using across many pages as this helps to prevent resource usage from creaping up.
    $session = $repository->createSession($ticket);
    
    return $session;
  }
  catch (Exception $e) {
    drupal_set_message('[alfresco_service_create_session] '. $e->getMessage(), 'error');
    watchdog('alfresco', $e->getMessage(), NULL, WATCHDOG_ERROR);
  }
}

/**
 * 
 * @param $session
 *  Session
 * @param $nodeRef
 *  Alfresco Node Reference
 * @return Store
 */
function alfresco_service_create_store($session, $nodeRef) {
  try {
    $infoRef = alfresco_parse_reference($nodeRef);
    return new Store($session, $infoRef['address'], $infoRef['scheme']);
  }
  catch (Exception $e) {
    drupal_set_message('[alfresco_service_create_store] '. $e->getMessage(), 'error');
    watchdog('alfresco', $e->getMessage(), NULL, WATCHDOG_ERROR);
  }
}

/**
 * 
 * @param $session Session
 * @param $nodeRef
 * @return unknown_type
 */
function alfresco_service_exists_node($nodeRef, $session = NULL) {
  try {
    if (is_null($session)) {
      $session = alfresco_service_create_session();
    }
    $infoRef = alfresco_parse_reference($nodeRef);
    $store = new Store($session, $infoRef['address'], $infoRef['scheme']);
    $nodes = $session->query($store, "ID:\"$nodeRef\"");
    return count($nodes) > 0;
  }
  catch (Exception $e) {
    drupal_set_message('[alfresco_service_exists_node] '. $e->getMessage(), 'error');
  }
}

/**
 * 
 * @param $session Session
 * @param $nodeRef
 * @return Node
 */
function alfresco_service_get_node($nodeRef, $session = NULL) {
  try {
    $infoRef = alfresco_parse_reference($nodeRef);

    if (is_null($session)) {
      $session = alfresco_service_create_session();
      dpm($session, "AAAA");
    }
    
    $store = new Store($session, $infoRef['address'], $infoRef['scheme']);
    $nodes = $session->query($store, "ID:\"$nodeRef\"");
    $count = count($nodes);
    
    if ($count > 0) {
      if ($count > 1) {
        $msg = "Alfresco module: Se ha encontrado más de un nodo con la misma referencia !reference";
        $vars = array('!reference' => $nodeRef);
        watchdog('alfresco', $msg, $vars, WATCHDOG_WARNING);
      }
      return $nodes[0];
    }
  }
  catch (Exception $e) {
    drupal_set_message('[alfresco_service_get_node] '. $e->getMessage(), 'error');
    watchdog('alfresco', $e->getMessage(), NULL, WATCHDOG_ERROR);
  }
}
