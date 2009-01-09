<?php
// $Id$

/**
 * @file
 *
 */

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
  $form['space']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('View content items'),
  );

  if (isset($form_state['storage']['alf_node'])) {
    $alf_node = $form_state['storage']['alf_node'];

    $items = array();
    foreach ($alf_node->children as $child) {
      if ($child->child->type == '{http://www.alfresco.org/model/content/1.0}content') {
        $items[strval($child->child)] = $child->child;
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
          '#default_value' => $item->cm_title,
        );
        $form['files']['body'][$index] = array(
          '#type' => 'textfield',
          '#size' => 20,
        );
      }

      // Put the titles into an array.
      $form['files']['import']['#tree'] = TRUE;
      $form['files']['title']['#tree'] = TRUE;
      $form['files']['body']['#tree'] = TRUE;

      // Store a copy of the list into a form value so we can compare it to what
      // they submit and not have to worry about files being added or removed
      // from the filesystem.
      $form['file_list'] = array(
        '#type' => 'value',
        '#value' => $itemlist,
      );

      $form['files']['#theme'] = 'alfresco_import_table';

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

function alfresco_import_form_validate($form, &$form_state) {

  if (isset($form_state['storage']['import_phase'])) {

  }
  else {
    if (empty($form_state['values']['reference'])) {
      form_set_error('reference', t('You must set an Alfresco Space Reference.'));
    }
    $alf_node = alfresco_service_load_node($form_state['values']['reference']);
    if (empty($alf_node) || $alf_node->type != '{http://www.alfresco.org/model/content/1.0}folder') {
      form_set_error('reference', t('Invalid Alfresco Space Reference.'));
    }
    else {
      $form_state['storage']['alf_node'] = $alf_node;
    }
  }
}

function alfresco_import_form_submit($form, &$form_state) {

  if (isset($form_state['storage']['import_phase'])) {
    //unset($form_state['storage']);
  }
  else {
    //$form_state['storage']['import_phase'] = TRUE;
  }
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
      array('data' => drupal_render($form['title'][$key])),
      array('data' => drupal_render($form['body'][$key])),
    );
    $rows[] = $row;
  }
  return theme('table', $header, $rows);
}
