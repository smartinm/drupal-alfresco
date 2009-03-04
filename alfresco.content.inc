<?php
// $Id$

/**
 * @file
 * Alfresco node content type implementation.
 *
 * Contents of this file
 * =====================
 * 1. Edit node form
 * 2. Validate node functions
 * 3. Submit node functions
 * 4. Database functions
 * 5. View functions
 * 6. External API
 */

// ------------------------------------------------------------------
// =1 Edit node form
// ------------------------------------------------------------------

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
    '#type' => module_exists('alfresco_browser') ? 'alfresco_browser' : 'textfield',
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
      '#maxlength' => 255,
      '#weight' => -5,
      '#attributes' => array('readonly' => 'readonly'),
      '#prefix' => '<div id="alfresco-edit-title-wrapper">',
      '#suffix' => '</div>',
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

// ------------------------------------------------------------------
// =2 Validate node functions
// ------------------------------------------------------------------

/**
 * Valida que la referencia al nodo de alfresco es correcta, comprueba que el nodo existe y obtiene el título.
 *
 * @see alfresco_form_submit
 * @see http://drupal.org/node/144132#passing
 */
function alfresco_form_validate($form, &$form_state) {

  if ($form_state['values']['reference']) {
    $reference = $form_state['values']['reference'];

    if (!alfresco_valid_reference($reference)) {
      form_set_error('reference', t('Alfresco Node Reference is in an invalid format.'));
    }
    else {
      // Check node in Alfresco repository.
      $alf_node = alfresco_service_load_node($reference);
      if ($alf_node) {
        $form_state['alf_node'] = $alf_node;
      }
      else {
        // No such node in Alfresco repository.
        $nid = $form_state['values']['nid'];
        if ($nid) {
          // Node edit
          $url_edit   = url('node/'. $nid .'/edit', array('fragment' => 'edit-reference-wrapper'));
          $url_delete = url('node/'. $nid .'/delete');

          form_set_error('reference',
            t('No such node in Alfresco repository. <a href="@url_edit">Edit Alfresco node reference</a> or <a href="@url_delete">delete this node</a>.',
              array('@url_edit' => $url_edit, '@url_delete' => $url_delete)));

          watchdog('alfresco', 'No such node in Alfresco repository.', NULL, WATCHDOG_NOTICE, l('view', 'node/'. $nid));
        }
        else {
          // Node new
          form_set_error('reference', t('No such node in Alfresco repository.'));
        }
      }
    }
  }
}

/**
 * Implementation of hook_validate().
 */
function alfresco_validate($node, &$form) {

  drupal_add_js(drupal_get_path('module', 'alfresco') .'/alfresco.js');

  if (!empty($node->reference)) {
    if ($nid = alfresco_exists_node($node->reference, $node->nid)) {
      form_set_error('reference', t('This Alfresco node already exists in your Drupal site. [<a href="!url">View</a>]',
        array('!url' => url('node/'. $nid))));
    }
  }
}

// ------------------------------------------------------------------
// =3 Submit node functions
// ------------------------------------------------------------------

/**
 * Submit handler for node.
 */
function alfresco_form_submit($form, &$form_state) {

  if (isset($form_state['alf_node'])) {
    $alf_node = $form_state['alf_node'];

    $properties = alfresco_service_load_properties($alf_node);
    if ($properties) {
      foreach ($properties as $key => $value) {
        $form_state['values'][$key] = $value;
      }

      if (!empty($form_state['values']['use_title'])) {
        $title = alfresco_truncate_title($properties['cm_title']);
        $form_state['values']['title'] = $title;
      }
    }

    unset($form_state['alf_node']);
  }
}

// -----------------------------------------------------------------------
// =4 Database functions
// -----------------------------------------------------------------------

/**
 * Implementation of hook_insert().
 */
function alfresco_insert($node) {
  _alfresco_save($node);
}

/**
 * Implementation of hook_update().
 *
 * As an existing node is being updated in the database, we need to do our own
 * database updates.
 */
function alfresco_update($node) {
  _alfresco_save($node);
}

/**
 * Save an alfresco node in database.
 */
function _alfresco_save($node) {

  // update timestamp
  $node->timestamp = time();

  // if this is a new node or we're adding a new revision
  if ($node->is_new || !empty($node->revision)) {
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
  else {
    db_query('DELETE FROM {alfresco_content} WHERE vid = %d', $node->vid);
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
    $alfresco_node->is_file = TRUE;
    $alfresco_node->cm_content = $content_data;
  }

  return $alfresco_node;
}

// -----------------------------------------------------------------------
// =5 View functions
// -----------------------------------------------------------------------

/**
 * Implementation of hook_view().
 *
 * This is a typical implementation that simply runs the node text through
 * the output filters.
 */
function alfresco_view($node, $teaser = FALSE, $page = FALSE) {

  $node = node_prepare($node, $teaser);
  if ($teaser) {
    $node->content['alfresco_info'] = array(
      '#value'  => theme('alfresco_teaser', $node),
      '#weight' => 0,
    );
  }
  else {
    $node->content['alfresco_info'] = array(
      '#value'  => theme('alfresco_display', $node),
      '#weight' => -1,
    );
  }
  return $node;
}

// ------------------------------------------------------------------
// =6 External API
// ------------------------------------------------------------------

/**
 * Synchronize the alfresco_item node with Alfresco Node.
 */
function alfresco_sync_node($node) {
  global $user;

  // check permissions
  if (!user_access('administer alfresco')) {
    return FALSE;
  }

  // check node content type
  if (!$node || $node->type != 'alfresco_item') {
    return FALSE;
  }

  // check alfresco node reference
  $reference = $node->reference;
  if (!alfresco_valid_reference($reference)) {
    return FALSE;
  }

  // check alfresco node in repository
  $alf_node = alfresco_service_load_node($reference);
  if (!$alf_node) {
    // @todo el nodo de Alfresco no existe, decidir que hacer.
    return FALSE;
  }

  // update properties
  $properties = alfresco_service_load_properties($alf_node);
  if (!$properties) {
    foreach ($properties as $key => $value) {
      $node->$key = $value;
    }

    // @todo mapping
    // $node->title = $properties->cm_title;
    // $node->body  = $properties->cm_description;
  }

  // save node
  node_save($node);

  return TRUE;
}

/**
 * Create an alfresco_item node from a reference.
 *
 * @param $reference
 *   String Alfresco Node Reference.
 * @param $title
 *   String to be used as the node's title. If this is ommitted the title of
 *   Alfresco item will be used.
 * @param $body
 *   String to be used as the node's body.
 * @param $options
 *   An associative array of additional options, with the following keys:
 *     - 'name'
 *       Authored by field. Leave blank to use the current user.
 *     - 'date'
 *       Authored on field. Leave blank to use the current time.
 *     - 'status'
 *       Published field.
 *     - 'promote'
 *       Promoted to front page field.
 *     - 'sticky'
 *       Sticky at top of lists field.
 *     - 'language'
 *       Language code if the locale.module is installed.
 *     - 'comment'
 *       Comment settings if the comment.module is installed.
 *     - 'taxonomy'
 *       Taxonomy terms to assign to the node if the taxonomy.module is
 *       installed.
 * @return
 *   A node object if the node is created successfully or FALSE on error.
 *
 * @see http://www.darrenmothersele.com/content/programmatically-creating-drupal-nodes
 * @see http://acquia.com/blog/migrating-drupal-way-part-i-creating-node
 * @see http://drupal.org/node/293663
 * @see http://drupal.org/node/178506#comment-895418
 * @see image_create_node_from()
 * @see drupal_execute()
 */
function alfresco_create_node($reference, $title = '', $body = '', $options = array()) {
  global $user;

  if (!user_access('create alfresco content')) {
    return FALSE;
  }

  // Ensure it's a valid alfresco node referente
  if (!alfresco_valid_reference($reference)) {
    return FALSE;
  }

  if (alfresco_exists_node($reference)) {
    return FALSE;
  }

  $alf_node = alfresco_service_load_node($reference);
  if (!$alf_node) {
    return FALSE;
  }

  // Build the alfresco_item node
  $node = new stdClass();
  $node->type  = 'alfresco_item';
  $node->name  = $user->name;
  $node->title = alfresco_truncate_title($title);
  $node->body  = $body;

  // Set the node's defaults
  module_load_include('inc', 'node', 'node.pages');
  node_object_prepare($node);

  // Node author information and options for administrators
  if (user_access('administer nodes')) {
    foreach (array('name', 'date', 'status', 'promote', 'sticky') as $key) {
      if (isset($options[$key])) {
        $node->$key = $options[$key];
      }
    }
  }

  // Locale settings
  if (module_exists('locale')) {
    if (isset($options['language'])) {
      $node->language = $options['language'];
    }
    else {
      global $language;
      $node->language = $language->language;
    }
  }

  // Taxonomy
  if (module_exists('taxonomy') && isset($options['taxonomy'])) {
    $node->taxonomy = $options['taxonomy'];
  }

  // Comment
  if (module_exists('comment') && isset($options['comment'])) {
    $node->comment = $options['comment'];
  }

  // Alfresco node stuff
  $node->reference = $reference;

  $properties = alfresco_service_load_properties($alf_node);
  if ($properties) {
    foreach ($properties as $key => $value) {
      $node->$key = $value;
    }
  }

  if (empty($title) && isset($node->cm_title)) {
    $node->title = alfresco_truncate_title($node->cm_title);
  }

  // Save the node.
  $node = node_submit($node);
  node_save($node);

  return $node;
}
