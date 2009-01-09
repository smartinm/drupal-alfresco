<?php
// $Id$

/**
 * @file
 *
 */

function alfresco_import_validate_item($item) {
  $errors = array();
  $reference = strval($item);
  if ($count = db_result(db_query("SELECT COUNT(*) FROM {alfresco_node} WHERE reference = '%s'", $reference))) {
    $errors[] = t('Already exists in your Drupal site.');
  }
  return $errors;
}

/**
 *
 */
function alfresco_import_form($form_state) {
  $form = array();

  $form['space'] = array(
    '#type' => 'fieldset',
    '#title' => t('Alfresco Space'),
    '#weight' => -10,
  );
  $form['space']['reference'] = array(
    '#type' => 'textfield',
    '#title' => t('Space Reference'),
    '#required' => TRUE,
    '#description' => t('Alfresco Space Reference. Example: workspace://SpacesStore/uuid'),
    '#default_value' => isset($form_state['values']['reference']) ? $form_state['values']['reference'] : '',
    '#attributes' => array('style' => 'width: 95%'),
  );
  $form['space']['load'] = array(
    '#type' => 'submit',
    '#value' => t('Load content items'),
    '#submit' => array('alfresco_import_load_space_submit'),
    '#validate' => array('alfresco_import_load_space_validate'),
  );

  if (isset($form_state['storage']['alf_node'])) {
    $alf_node = $form_state['storage']['alf_node'];

    $items = array();
    foreach ($alf_node->children as $child) {
      if ($child->child->type == '{http://www.alfresco.org/model/content/1.0}content') {
        $items[] = $child->child;
      }
    }

    if ($items) {
      $form['#node_type'] = 'alfresco_item';

      if (module_exists('taxonomy')) {
        // here's a little hack to get the taxonomy controls onto our form
        $form['type'] = array('#type' => 'value', '#value' => $form['#node_type']);
        $form['#node'] = new stdClass();
        $form['#node']->type = $form['#node_type'];
        taxonomy_form_alter($form, array(), $form['#node_type'] .'_node_form');
        unset($form['type']);
        unset($form['#node']);

        $form['taxonomy']['#type'] = 'fieldset';
        $form['taxonomy']['#title'] = t('Vocabularies');
        $form['taxonomy']['#collapsible'] = TRUE;
        $form['taxonomy']['#collapsed'] = FALSE;
        $form['taxonomy']['#weight'] = 5;
      }

      // Put the alfresco items into an array for the checkboxes and gather
      // additional information.
      // Make sure that there's no 0th element, because a checkbox with a zero
      // value is seen as unchecked and won't be imported.
      $index = 0;
      foreach ($items as $item) {
        $index++;
        $itemlist[$index] = strval($item);

        $problems = alfresco_import_validate_item($item);

        // Spit out the import form elements.
        $form['files']['import'][$index] = array(
          '#type' => 'checkbox',
          '#title' => $item->cm_name,
        );
        $form['files']['filesize'][$index] = array(
          '#type' => 'item',
          '#value' => format_size($item->cm_content->size),
        );
        $form['files']['title'][$index] = array(
          '#type' => 'textfield',
          '#size' => 20,
          '#maxlength' => 255,
          '#default_value' => $item->cm_title,
        );
        $form['files']['body'][$index] = array(
          '#type' => 'textfield',
          '#size' => 20,
        );

        // If there were problems don't let them import it
        if (count($problems)) {
          $form['files']['import'][$index]['#type'] = 'item';
          $form['files']['errors'][$index] = array(
            '#type' => 'markup',
            '#value' => '<em>'. implode(' ', $problems) .'</em>',
          );
          unset($form['files']['title'][$index]);
          unset($form['files']['body'][$index]);
        }
      }

      // Put the titles into an array.
      $form['files']['import']['#tree'] = TRUE;
      $form['files']['title']['#tree'] = TRUE;
      $form['files']['body']['#tree'] = TRUE;

      // Table theme
      $form['files']['#theme'] = 'alfresco_import_table';

      // Store a copy of the list into a form value so we can compare it to what
      // they submit and not have to worry about files being added or removed
      // from the filesystem.
      $form['file_list'] = array(
        '#type' => 'value',
        '#value' => $itemlist,
      );

      $form['buttons']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Import'),
      );
      $form['buttons']['reset'] = array(
        '#type'   => 'markup',
        '#value'  => '<input '. drupal_attributes(array('type' => 'button', 'value' => t('Reset'))) .' onClick="window.location=\''. url('admin/content/alfresco_import') .'\';" />',
      );
      $form['buttons']['#weight'] = 10;
    }
    else {
      $form['none_found'] = array(
        '#type' => 'item',
        '#value' => '<em>'. t('No Alfresco content items were found.') .'</em>',
      );
    }
  }

  return $form;
}

function alfresco_import_load_space_validate($form, &$form_state) {
  dpm('alfresco_import_load_space_validate');

  if (empty($form_state['values']['reference'])) {
    form_set_error('reference', t('You must set an Alfresco Space Reference.'));
  }
  else {
    $alf_node = alfresco_service_load_node($form_state['values']['reference']);
    if (empty($alf_node)) {
      form_set_error('reference', t('Invalid Alfresco Space Reference.'));
    }
    elseif ($alf_node->type != '{http://www.alfresco.org/model/content/1.0}folder') {
      form_set_error('reference', t('Alfresco Reference is not a Space.'));
    }
    else {
      $form_state['storage']['alf_node'] = $alf_node;
    }
  }
}

function alfresco_import_load_space_submit($form, &$form_state) {
  dpm('alfresco_import_load_space_submit');
}

function alfresco_import_form_validate($form, &$form_state) {
  dpm('alfresco_import_form_validate');
}

function alfresco_import_form_submit($form, &$form_state) {
  dpm('alfresco_import_form_submit');

  $batch = array(
    'title' => t('Importing alfresco content'),
    'progress_message' => 'Importing @current of @total.',
    'operations' => array(),
    'finished' => '_alfresco_import_batch_finished',
    'file' => drupal_get_path('module', 'alfresco_import') .'/alfresco_import.pages.inc',
  );

  foreach (array_filter($form_state['values']['import']) as $index => $true) {
    $reference = $form_state['values']['file_list'][$index];
    if (TRUE) {
      $args = array(
        'node_type' => $form['#node_type'],
        'title' => isset($form_state['values']['title'][$index]) ? $form_state['values']['title'][$index] : NULL,
        'body' => isset($form_state['values']['body'][$index]) ? $form_state['values']['body'][$index] : NULL,
        'taxonomy' => isset($form_state['values']['taxonomy']) ? $form_state['values']['taxonomy'] : array(),
        'reference' => $reference,
      );
      $batch['operations'][] = array('_alfresco_import_batch_op', array($args));
    }
  }

  unset($form_state['storage']['alf_node']);

  batch_set($batch);
}

function _alfresco_import_batch_op($args, &$context) {

  // Create the node object.
  if ($node = alfresco_create_node($args['reference'], TRUE, $args['title'], $args['body'], $args['taxonomy'])) {

    $context['results']['good'][] = t('Imported %reference as <a href="!node-link">@node-title</a> @status <a href="!edit-link">[edit]</a>.', array(
      '%reference' => $args['reference'],
      '!node-link' => url('node/'. $node->nid),
      '@node-title' => $node->title,
      '@status' => $node->status ? '' : t('(Unpublished)'),
      '!edit-link' => url('node/'. $node->nid .'/edit'),
    ));
  }
  else {
    watchdog('alfresco_import', 'There was an error that prevented %reference from being imported.', array('%reference' => $args['reference']), WATCHDOG_ERROR);
    $context['results']['bad'][] = t('Error importing %reference.', array('%reference' => $args['reference']));
  }

  $context['finished'] = 1;
}

function _alfresco_import_batch_finished($success, $results, $operations) {
  if (!$success) {
    if (isset($results['bad']) && count($results['bad'])) {
      drupal_set_message(t('There was a problem importing files: !bad-list', array('!bad-list' => theme('item_list', $results['bad']))), 'error');
    }
    else {
      drupal_set_message(t('There was a problem importing the files.'), 'error');
    }
  }
  if (isset($results['good']) && count($results['good'])) {
    drupal_set_message(t('Successfully imported: !good-list', array('!good-list' => theme('item_list', $results['good']))));
  }
  watchdog('alfresco_import', 'Completed image import.');
}

function theme_alfresco_import_table($form) {

  $type = node_get_types('type', 'alfresco_item');
  $header = array(theme('table_select_header_cell'), t('Name'), t('Size'), check_plain($type->title_label), check_plain($type->body_label));
  $rows = array();
  foreach (element_children($form['import']) as $key) {
    $filename = $form['import'][$key]['#title'];
    unset($form['import'][$key]['#title']);
    $row = array(
      array('data' => drupal_render($form['import'][$key])),
      array('data' => $filename),
      array('data' => drupal_render($form['filesize'][$key])),
    );
    if (!isset($form['errors'][$key])) {
      $row[] = array('data' => drupal_render($form['title'][$key]));
      $row[] = array('data' => drupal_render($form['body'][$key]));
    }
    else {
      $row[] = array('colspan' => 2, 'data' => drupal_render($form['errors'][$key]));
    }
    $rows[] = $row;
  }
  return theme('table', $header, $rows);
}
