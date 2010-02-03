<?php
// $Id$

/**
 * @file
 * Provides functions for xxx
 */

/**
 *
 */
function alfresco_search_name() {
  return t('Alfresco');
}

/**
 * @todo: Mover la lógica de las búsquedas a un método de alfresco.service.inc
 */
function alfresco_search_search($keys) {
  $find = array();

  try {
    alfresco_include('soap');

    $session = alfresco_soap_get_session();
    $spacesStore = new AlfStore($session, 'SpacesStore');
    $results = $session->query($spacesStore, '+ISNODE:T +TYPE:"cm:content" +ALL:"*'. $keys .'*"');

    foreach ($results as $node) {
      $reference = $node->__toString();
      $nid = alfresco_exists_node($reference);

      if ($node->cm_content instanceof AlfContentData) {
        $content = $node->cm_content;
      }

      $name = $node->cm_name;
      $title = $node->cm_title;
      $description = $node->cm_description;
      $extension = substr(strrchr($node->cm_name, '.'), 1);
      $type = $node->type;
      //$path = $node->getFolderPath() . '/'. $name;

      $store = $node->getStore();
      $link = "alfresco/download/$store->scheme/$store->address/$node->id/$node->cm_name";

      $find[] = array(
        'link'    => url($link),
        'title'   => !empty($title) ? $title : $name,
        //'snippet' => $description,
        'user'    => $node->cm_creator,
        'extra'   => array(
          'size'     => isset($content) ? format_size($content->size) : '',
          'mimetype' => isset($content) ? $content->mimetype : '',
        ),
      );
    }
  }
  catch (SoapFault $exception) {
    alfresco_error_handler('alfresco', $exception);
  }

  return $find;
}

/**
 *
 */
function alfresco_search_form_alter(&$form, &$form_state) {

  if ($form['module']['#value'] == 'alfresco') {
    // @todo: Implements advanced alfresco search
  }
}
