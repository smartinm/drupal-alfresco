<?php
// $Id$

/**
 * @file
 * Implementación de las operaciones CRUD sobre el tipo de contenido.
 */

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
    '#default_value' => isset($node->reference) ? $node->reference : '',
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
      //'#disabled' => TRUE,
      '#attributes' => array('readonly' => 'readonly'),
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
  $form['#submit'][]   = 'alfresco_form_submit';

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function alfresco_validate($node, &$form) {
  apm("alfresco_validate");
  
  // Validate uri.
  if (!empty($node->reference)) {
    if ($nid = db_result(db_query("SELECT nid FROM {alfresco_node} WHERE reference = '%s' AND nid <> %d", $node->reference, $node->nid))) {
      form_set_error('reference', t('Ya existe otro contenido referenciando este nodo de Alfresco (<a href="!url">ir al contenido</a>)', array('!url' => url('node/'. $nid))));
    }
  }
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
  apm("alfresco_form_validate");
  
  if ($form_state['values']['reference']) {
    $nodeRef = $form_state['values']['reference'];
    
    if (!alfresco_valid_reference($nodeRef)) {
      form_set_error('reference', t('La referencia al nodo de Alfresco no tiene el formato correcto.'));
    }
    else {
      $alf_node = alfresco_service_load_node($nodeRef);
      if (!$alf_node) {
        form_set_error('reference', t('La referencia al nodo de Alfresco no existe.'));
        // @TODO Añadir opción para eliminar el nodo en Drupal
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
  apm("alfresco_form_submit");

  if (isset($form_state['alf_node'])) {
    $alf_node = $form_state['alf_node'];
    
    $form_state['values']['cm_name']        = $alf_node->cm_name;
    $form_state['values']['cm_title']       = $alf_node->cm_title;
    $form_state['values']['cm_description'] = $alf_node->cm_description;
    $form_state['values']['cm_author']      = $alf_node->cm_author;
    $form_state['values']['cm_created']     = $alf_node->cm_created;
    $form_state['values']['cm_creator']     = $alf_node->cm_creator;
    $form_state['values']['cm_modified']    = $alf_node->cm_modified;
    $form_state['values']['cm_modifier']    = $alf_node->cm_modifier;
    $form_state['values']['cm_author']      = $alf_node->cm_author;
    
    if (!empty($form_state['values']['use_title'])) {
      $form_state['values']['title'] = $alf_node->cm_title;
    }
    
    if (is_object($alf_node->cm_content)) {
      $data = $alf_node->cm_content;

      $content = array();
      $content['url']      = $data->url;
      $content['mimetype'] = $data->mimetype;
      $content['size']     = $data->size;
      $content['encoding'] = $data->encoding;
       
      $form_state['values']['cm_content'] = $content;
    }
  }
  else {
    // @TODO el nodo de alfresco se ha eliminado
  }
}

// -----------------------------------------------------------------------
// Alfresco database functions 
// -----------------------------------------------------------------------

/**
 * Implementation of hook_insert().
 */
function alfresco_insert($node) {
  alfresco_save($node);
}

/**
 * Implementation of hook_update().
 *
 * As an existing node is being updated in the database, we need to do our own
 * database updates.
 */
function alfresco_update($node) {
  alfresco_save($node);
}

function alfresco_save($node) {

  // if this is a new node or we're adding a new revision,
  if ($node->is_new || $node->revision) {
    drupal_write_record('alfresco_node', $node);
  }
  else {
    drupal_write_record('alfresco_node', $node, 'vid');
  }

  if (isset($node->cm_content)) {
    $content = $node->cm_content;
    $content['vid'] = $node->vid;
    $content['nid'] = $node->nid;
    
    drupal_write_record('alfresco_content', $content, 'vid');
    if (!db_affected_rows()) {
      drupal_write_record('alfresco_content', $content);
    }
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
      db_query('DELETE FROM {alfresco_node} WHERE vid = %d', $node->vid);
      db_query('DELETE FROM {alfresco_content} WHERE vid = %d', $node->vid);
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
  db_query('DELETE FROM {alfresco_node} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {alfresco_content} WHERE nid = %d', $node->nid);
}

/**
 * Implementation of hook_load().
 *
 * Now that we've defined how to manage the node data in the database, we
 * need to tell Drupal how to get the node back out. This hook is called
 * every time a node is loaded, and allows us to do some loading of our own.
 */
function alfresco_load($node) {
  
  $fields = drupal_schema_fields_sql('alfresco_node');
  $fields = array_diff($fields, array('vid', 'nid'));
  $fields = implode(', ', $fields);

  $sql = "SELECT $fields FROM {alfresco_node} WHERE vid = %d";
  $alfresco_node = db_fetch_object(db_query($sql, $node->vid));

  $fields = drupal_schema_fields_sql('alfresco_content');
  $fields = array_diff($fields, array('vid', 'nid'));
  $fields = implode(', ', $fields);
  
  $sql = "SELECT $fields FROM {alfresco_content} WHERE vid = %d";
  $content_data = db_fetch_array(db_query($sql, $node->vid));
  if ($content_data) {
    $alfresco_node->cm_content = $content_data;
  }
  
  return $alfresco_node;
}

/**
 * Implementation of hook_view().
 *
 * This is a typical implementation that simply runs the node text through
 * the output filters.
 */
function alfresco_view($node, $teaser = FALSE, $page = FALSE) {
  
  $node = node_prepare($node, $teaser);

  $mapping = array(
    'name'        => t('Name'),
    'title'       => t('Title'),
    'description' => t('Description'),
    'content'     => array(
      'mimetype'  => t('Mimetype'),
      'encoding'  => t('Encoding'),
      'size'      => t('Size'),
    ),
    'author'      => t('Author'),
    'creator'     => t('Creator'),
    'created'     => t('Created date'),
    'modifier'    => t('Modifier'),
    'modified'    => t('Modified date'),
  );
  
  $properties = array();
  foreach ($mapping as $key => $name) {
    $property = 'cm_'. $key;
    if (isset($node->$property)) {
      $value = empty($node->$property) ? t('n/a') : $node->$property;
      
      if ($key == 'name' && isset($node->cm_content)) {
        $url = $node->cm_content['url'];
        $properties[] = array($name, l($value, $url, array('attributes' => array('title' => t('Download file')))));
      }
      elseif ($key == 'content') {
        $content = $node->cm_content;
        $properties[] = array($mapping['content']['mimetype'], $content['mimetype']);
        $properties[] = array($mapping['content']['size'], alfresco_bytestostring($content['size'], 2));
        $properties[] = array($mapping['content']['encoding'], $content['encoding']);
      }
      elseif ($key == 'created' || $key == 'modified') {
        $properties[] = array($name, format_date(strtotime($value), 'large'));
      }
      else {
        $properties[] = array($name, $value);
      }
    }
  }
  
  $header = array(t('Property'), t('Value'));
  $table_properties = theme('table', $header, $properties, NULL); 
  
  $node->content['alfresco_properties'] = array(
    '#value' => $table_properties,
    '#weight' => 1,
  );
  return $node;
  
  
  
  // Obtenemos el nodo de Alfresco a partir de su referencia.
  $alf_node = alfresco_service_load_node($node->reference);
  if (!$alf_node) {
    return $node;
  }

  /* 
  dpm($alf_node->getType());
  dpm($alf_node->getAspects());
  dpm($alf_node->getProperties());
  dpm($alf_node->getVersionHistory());
  dpm($alf_node->getAssociations());
  */
  
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
    
    /*
    dpm($contentData->getNode(), 'getNode');
    dpm($contentData->getProperty(), 'getProperty');
    dpm($contentData->getIsDirty(), 'getIsDirty');
    dpm($contentData->getMimetype(), 'getMimetype');
    dpm($contentData->getSize(), 'getSize');
    dpm($contentData->getEncoding(), 'getEncoding');
    dpm($contentData->getUrl(), 'getUrl');
    dpm($contentData->getGuestUrl(), 'getGuestUrl');
    dpm($contentData->getContent(), 'getContent');
    */
    
    
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

  $header = array(t('Property'), t('Value'));
  $table_properties = theme('table', $header, $properties, NULL); 
  
  $node->content['alfresco_properties'] = array(
    '#value' => $table_properties,
    '#weight' => 1,
  );
  
  return $node;
}



/**
 * Function to other modules to use to create image nodes.
 *
 * @param $reference
 *   String Alfresco Node Reference. 
 * @param $title
 *   String to be used as the node's title. If this is ommitted the filename 
 *   will be used.
 * @param $body 
 *   String to be used as the node's body.
 * @param $taxonomy
 *   Taxonomy terms to assign to the node if the taxonomy.module is installed.
 * @return 
 *   A node object if the node is created successfully or FALSE on error.
 * 
 * @see http://www.darrenmothersele.com/content/programmatically-creating-drupal-nodes
 * @see http://drupal.org/node/293663
 */
function alfresco_create_node_from($reference, $title = NULL, $body = '', $taxonomy = NULL) {
  global $user;

  if (!user_access('create alfresco content')) {
    return drupal_access_denied();
  }
  
  // Ensure it's a valid alfresco node referente.
  if (!alfresco_valid_reference($reference)) {
    return FALSE;
  }

  // Import node functions.
  module_load_include('inc', 'node', 'node.pages');

  // Create a new alfresco_item node
  $node = array(
    'type' => 'alfresco_item',
    'name' => $user->name,
    'taxonomy' => $taxonomy,
  );
  
  $form_state = array();
  $form_state['values']['title'] = is_null($title) ? '' : $title;
  $form_state['values']['body']  = $body;
  
  $form_state['values']['reference'] = $reference;
  $form_state['values']['use_title'] = TRUE;
  
  $form_state['values']['op'] = t('Save');
  
  drupal_execute('alfresco_item_node_form', $form_state, (object)$node);
}
