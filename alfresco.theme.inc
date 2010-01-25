<?php
// $Id$

/**
 * @file
 * Theme functions for Alfresco module.
 */

/**
 * Format the teaser for an alfresco_item node.
 */
function theme_alfresco_teaser($node) {

  $url = alfresco_url_download($node);
  $link = l($node->cm_name, $url, array('attributes' => array('title' => t('Download file'))));
  return '<p>'. $link .'</p>';
}

/**
 * Format the alfresco_item node for display.
 */
function theme_alfresco_display($node) {

  $model = array(
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

  /*
    'node-uuid'        => t('Node UUID'),
    'node-dbid'        => t('Node DBID'),
    'store-protocol'   => t('Store protocol'),
    'store-identifier' => t('Store identifier'),
   */

  $properties = array();
  foreach ($model as $key => $name) {
    $property = 'cm_'. $key;
    if (isset($node->$property)) {
      $value = empty($node->$property) ? t('n/a') : $node->$property;

      if ($key == 'name' && isset($node->cm_content) && user_access('download alfresco content')) {
        $url = alfresco_url_download($node);
        $properties[] = array($name, l($value, $url, array('attributes' => array('title' => t('Download file')))));
      }
      elseif ($key == 'content') {
        $content = $node->cm_content;
        $properties[] = array($model['content']['mimetype'], $content['mimetype']);
        $properties[] = array($model['content']['size'], format_size($content['size']));
        $properties[] = array($model['content']['encoding'], $content['encoding']);
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

  /*
  apm($alf_node->getChildren());
  apm($alf_node->getParents());
  apm($alf_node->getPrimaryParent());
  */

  return $table_properties;
}

/**
 * @todo documentation
 */
function theme_alfresco($element) {
  return drupal_render($element);
}
