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

  // Obtenemos el nodo de Alfresco a partir de su referencia.
  $alf_node = alfresco_service_get_node($node->reference);
  if (is_null($alf_node)) {
    return $node;
  }
  
  
  dpm($alf_node->getType());
  dpm($alf_node->getAspects());
  dpm($alf_node->getProperties());
  dpm($alf_node->getVersionHistory());
  dpm($alf_node->getAssociations());
  
  
  //
  // Alfresco node properties
  //
  $model_content = '{http://www.alfresco.org/model/content/1.0}';
  $mapping = array(
    'name'        => t('Name'),
    'title'       => t('Title'),
    'description' => t('Description'),
    'author'      => t('Author'),
    'creator'     => t('Creator'),
    'created'     => t('Created date'),
    'modifier'    => t('Modifier'),
    'modified'    => t('Modified date'),
  
    'node-uuid'        => t('Node UUID'),
    'node-dbid'        => t('Node DBID'),
    'store-protocol'   => t('Store protocol'),
    'store-identifier' => t('Store identifier'),
  
    'content'     => t('Content data'),
  
    'mimetype'  => t('Mimetype'),
    'encoding'  => t('Encoding'),
    'size'      => t('Size'),
    'url'       => t('URL'),
    'guest-url' => t('Guest URL'),
  );
  
  
  $contentData = $alf_node->cm_content;
  if (!is_null($contentData)) {
    
    dpm($contentData->getNode(), 'getNode');
    dpm($contentData->getProperty(), 'getProperty');
    dpm($contentData->getIsDirty(), 'getIsDirty');
    dpm($contentData->getMimetype(), 'getMimetype');
    dpm($contentData->getSize(), 'getSize');
    dpm($contentData->getEncoding(), 'getEncoding');
    dpm($contentData->getUrl(), 'getUrl');
    dpm($contentData->getGuestUrl(), 'getGuestUrl');
    dpm($contentData->getContent(), 'getContent');
    
    
    $size = $contentData->getSize();
    $url  = $contentData->getUrl();
    
    $file_properties = array(
      'mimetype'  => $contentData->getMimetype(),
      'encoding'  => $contentData->getEncoding(),
      'size'      => is_numeric($size) ? alfresco_bytestostring($size, 2) : $size,
      //'url'       => $url,
      //'guest-url' => $contentData->getGuestUrl(),
    );
  }
  
  $properties = array();
  foreach ($alf_node->getProperties() as $key => $value) {
    if(is_string($value) || is_null($value)) {
      $key = str_replace($model_content, '', $key);
      if (isset($mapping[$key])) {
        $value = is_null($value) ? t('n/a') : $value;
        $properties[] = array($mapping[$key], $value);  
      } 
      else {
        $properties[] = array($key, $value);
      }
    }
  }
  
  if (count($file_properties)) {
    foreach ($file_properties as $key => $value) {
      if(is_string($value) || is_null($value)) {
        if (isset($mapping[$key])) {
          $value = is_null($value) ? t('n/a') : $value;
          $properties[] = array($mapping[$key], $value);  
        }
      }
    }
  }

  $header = array(t('Propiedad'), t('Valor'));
  $table_properties = theme('table', $header, $properties, NULL, t('Propiedades')); 
  
  $node->content['alfresco_properties'] = array(
    '#value' => $table_properties,
    '#weight' => 1,
  );
  
  //
  // Alfresco content data
  //
  
  
  $header = array(t('Propiedad'), t('Valor'));
  $table_properties = theme('table', $header, $properties, NULL, t('Propiedades del nodo')); 
  
  $node->content['alfresco_properties'] = array(
    '#value' => $table_properties,
    '#weight' => 1,
  );

  return $node;
    
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
  $uuid_pattern = '[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}';
  return preg_match("/^\w+:\/\/\w+\/$uuid_pattern$/", $reference);
}

/**
 * 
 */
function alfresco_parse_reference($reference) {
  
  $info = array();

  $uuid_pattern = '[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}';
  if (preg_match("/^(\w+):\/\/(\w+)\/($uuid_pattern)$/", $reference, $matches)) {
    
    $info['scheme']  = $matches[1];
    $info['address'] = $matches[2];
    $info['uuid']    = $matches[3];
  }
  return $info;
}

/**
 * Present a size (in bytes) as a human-readable value
 *
 * @param int    $size        size (in bytes)
 * @param int    $precision    number of digits after the decimal point
 * @return string
 * 
 * @see http://php.net/manual/es/function.filesize.php#84652
 */
function alfresco_bytestostring($size, $precision = 0) {
    $sizes = array('YB', 'ZB', 'EB', 'PB', 'TB', 'GB', 'MB', 'kB', 'B');
    $total = count($sizes);

    while($total-- && $size > 1024) $size /= 1024;
    return round($size, $precision) .' '. $sizes[$total];
}

function alfresco_service_create_session() {

  try {
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
    $msg = "Error iniciando sesión con el repositorio de Alfresco";
    drupal_set_message(t($msg), 'error');
    
    if ($e->getMessage()) {
      watchdog('alfresco', $e->getMessage(), NULL, WATCHDOG_ERROR,
        l(t('Alfresco repository'), $repositoryUrl));
    }
  }
}

/**
 * 
 * @param $session Session
 * @param $node_ref
 * @return Node
 */
function alfresco_service_get_node($node_ref, $session = NULL) {

  $info_ref = alfresco_parse_reference($node_ref);
  
  try {
    if (is_null($session)) {
      $session = alfresco_service_create_session();
    }

    $store = $session->getStore($info_ref['address'], $info_ref['scheme']);
    $nodes = $session->query($store, 'ID:"'. $node_ref .'"');
    
    if ($count = count($nodes)) {
      if ($count > 1) {
        $msg = "Se ha encontrado más de un nodo con referencia !reference";
        $vars = array('!reference' => $node_ref);
        
        watchdog('alfresco', $msg, $vars, WATCHDOG_WARNING,
          l(t('Alfresco node reference'), $node_ref));
      }
      return $nodes[0];
    }
  }
  catch (Exception $e) {
    $msg = "Error obteniendo el nodo de Alfresco con referencia !reference";
    $args = array('!reference' => $node_ref);
    drupal_set_message(t($msg, $args), 'error');
    
    if ($e->getMessage()) {
      watchdog('alfresco', $e->getMessage(), NULL, WATCHDOG_ERROR,
        l(t('Alfresco node reference'), $node_ref));
    }
  }
}
