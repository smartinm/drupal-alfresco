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
 * @todo: Mover la búsqueda a alfresco.service.inc
 */
function alfresco_search_search($keys) {
  $find = array();

  try {
    alfresco_include('soap');
    $session = alfresco_soap_get_session();
    $spacesStore = new AlfStore($session, 'SpacesStore');
    $queryResults = $session->query($spacesStore, '+ISNODE:T +TYPE:"cm:content" +ALL:"*'. $keys .'*"');

    foreach ($queryResults as $childNode) {
      $nodeRef = $childNode->__toString();
      $ext = substr(strrchr($childNode->cm_name, '.'), 1);
      $nid = alfresco_exists_node($nodeRef);

      if ($childNode->cm_content instanceof AlfContentData) {
        $contentData = $childNode->cm_content;
      }

      $name = $childNode->cm_name;
      $title = $childNode->cm_title;
      $description = $childNode->cm_description;

      $find[] = array(
        'link' => url('alfresco/download/'. $childNode->id .'/'. $childNode->cm_name),
        //'type' => $childNode->type,
        'title' => !empty($title) ? $title : $name,
        'user' => $childNode->cm_creator,
        'extra' => array(
          //'path' => $childNode->getFolderPath() . '/'. $name,
          'size' => isset($contentData) ? format_size($contentData->size) : '',
          'mimetype' => isset($contentData) ? $contentData->mimetype : '',
        ),
        'snippet' => $description,
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
