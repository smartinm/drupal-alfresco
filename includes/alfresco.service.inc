<?php
// $Id$

/**
 * @file alfresco.service.inc
 * Provides functions for access to Alfresco repository through of web service.
 */

/**
 * Devuelve un objeto AlfSession que permite realizar peticiones a Alfresco.
 *
 * @return AlfSession
 */
function alfresco_service_get_session() {
  static $session = NULL;
  if (!isset($session)) {
    $session = alfresco_service_create_session();
  }
  return $session;
}

/**
 * Authenticate against the repository and create a new session.
 *
 * @return Session
 *
 * @see http://wiki.alfresco.com/wiki/PHP_Tutorial_One
 * @see http://wiki.alfresco.com/wiki/PHP_Tutorial_One_Complete_Script
 */
function alfresco_service_create_session() {
  $session = NULL;

  alfresco_service_include('Repository');

  try {
    // The web service end point URL for the Alfresco repository
    $repository_url = variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api');

    // Create the repository object baseed on the repository URL.  The
    // repository represents the Alfresco repository we are connecting to.
    // It can be used to authenticate users and as a session factory.
    $repository = new AlfRepository($repository_url);

    // Authenticate the user details for the repository we want to connect to.
    // Assuming authentication is succesful we will be returned a ticket.
    // The ticket is a string which can be used to create Session's
    $ticket = _alfresco_service_authenticate($repository);

    // Using the ticket we can now create a session.  In general it is better
    // to create a new session for a new page or page sequence rather than
    // creating one and using across many pages as this helps to prevent
    // resource usage from creaping up.
    $session = $repository->createSession($ticket);
  }
  catch (SoapFault $exception) {
    alfresco_error_handler('alfresco', $exception);
  }

  return $session;
}

/**
 * Load an alfresco node object from the Alfresco repository.
 *
 * @param $reference_or_path
 *  Either the node reference of the Alfresco node or the path to the Alfresco
 *  node (trail of QNames of the child relationships to get to the node).
 * @param $session Session
 *  xxx
 * @param $reset
 *  Whether to reset the internal node cache.
 *
 * @return AlfNode
 *  A fully-populated Alfresco node object.
 */
function alfresco_service_node_load($reference_or_path, $reset = NULL) {
  static $nodes = array();

  if ($reset) {
    $nodes = array();
  }

  if (!$inforef = alfresco_parse_reference($reference_or_path)) {
    if (!alfresco_valid_path($reference_or_path)) {
      return FALSE;
    }
  }

  if ($inforef) {
    $ref = $inforef['reference'];
    // Is the node statically cached?
    if (isset($nodes[$ref])) {
      return is_object($nodes[$ref]) ? drupal_clone($nodes[$ref]) : $nodes[$ref];
    }
  }

  try {
    if (!$session = alfresco_service_get_session()) {
      return FALSE;
    }

    if ($inforef) {
      // @see https://issues.alfresco.com/jira/browse/AR-1063
      $store = new AlfStore($session, $inforef['address'], $inforef['scheme']);
      $nodes = $session->query($store, '+ISNODE:T +ID:"'. $inforef['reference'] .'"');
    }
    else {
      $store = new AlfStore($session, 'SpacesStore');
      $nodes = $session->query($store, 'PATH:"'. $reference_or_path .'"');
    }

    if (count($nodes)) {
      $node = $nodes[0];

      if ($inforef) {
        $ref = $inforef['reference'];
        $nodes[$ref] = is_object($node) ? drupal_clone($node) : $node;
      }

      return $node;
    }
  }
  catch (SoapFault $exception) {
    alfresco_error_handler('alfresco', $exception);
  }
}

/**
 *
 * @param $reference_or_path
 * @param $session
 * @param $reset
 * @return unknown_type
 */
function alfresco_service_node_delete($reference_or_path) {

  if (!$inforef = alfresco_parse_reference($reference_or_path)) {
    if (!alfresco_valid_path($reference_or_path)) {
      return FALSE;
    }
  }

  try {
    if (!$session = alfresco_service_get_session()) {
      return FALSE;
    }

    if ($inforef) {
      // @see https://issues.alfresco.com/jira/browse/AR-1063
      $store = new AlfStore($session, $inforef['address'], $inforef['scheme']);
      $nodes = $session->query($store, '+ISNODE:T +ID:"'. $inforef['reference'] .'"');
    }
    else {
      $store = new AlfStore($session, 'SpacesStore');
      $nodes = $session->query($store, 'PATH:"'. $reference_or_path .'"');
    }

    if (count($nodes)) {
      $node = $nodes[0];

      $statements = array(
        "delete" => array(
          "where" => array(
            "nodes" => $node->__toArray()
          )
        )
      );

      $session->repositoryService->update(array('statements' => $statements));

      return TRUE;
    }
  }
  catch (SoapFault $exception) {
    alfresco_error_handler('alfresco', $exception);
  }
}

/**
 * Wrapper for Lucene API.
 *
 * Examples of valid paths:
 * - "/"
 * - "/*"
 * - "/app:*"
 * - "/app:company_home"
 * - "/app:company_home/*"
 * - "/app:company_home//*"
 * - "/app:company_home//."
 * - "//app:company_home"
 * - "//app:company_home//*"
 *
 * @see http://wiki.alfresco.com/wiki/Search
 */
function alfresco_service_search($text, $property = '@cm:name') {

  try {
    if (!$session = alfresco_service_get_session()) {
      return FALSE;
    }

    // Lucene requires the : to be escaped using the \ character
    $property = !empty($property) ? str_replace(':', '\:', $property) : 'ALL';
    $query = '+ISNODE:T +TYPE:"cm:content" +'. $property .':"*'. $text .'*"';

    $spacesStore = new AlfSpacesStore($session);
    $queryResults = $session->query($spacesStore, $query);

    //$nodes = $session->query($store, 'PATH:"/app:company_home/*"');

    return $queryResults;
  }
  catch (SoapFault $exception) {
    alfresco_error_handler('alfresco', $exception);
  }
}

/**
 * Uploads a file into content store and returns the content data string which
 * can be used to populate a content property.
 *
 * @param $session the session
 * @param $filepath the file location
 * @param $mimetype
 * @param $encoding
 * @param $properties
 *  cm_title
 *  cm_description
 *  cm_author
 * @return String the content data that can be used to update the content property
 *
 * @see alf_upload_file()
 * @see http://wiki.alfresco.com/wiki/URL_Addressability#UploadContentServlet
 */
function alfresco_service_node_add($space, $filepath, $filename = NULL, $mimetype = NULL, $encoding = 'UTF-8', $properties = array()) {

  try {
    if (!$session = alfresco_service_get_session()) {
      return FALSE;
    }

    $space_node = alfresco_service_node_load($space);
    if (!$space_node) {
      return FALSE;
    }

    // Get the address and the port
    $host = $session->repository->host;
    $port = $session->repository->port;

    if (empty($filename)) {
      $filename = basename($filepath);
    }

    // Write the request header onto the socket
    $url = 'http://'. $host .':'. $port .'/alfresco/upload/'. urlencode($filename) .'?ticket='. $session->ticket;

    if (!empty($mimetype)) {
      // Add mimetype if specified
      $url .= '&mimetype='. $mimetype;
    }

    if (!empty($encoding)) {
      // Add encoding if specified
      $url .= '&encoding='. $encoding;
    }

    $result = alfresco_file_put($filepath, $url);

    if ($result->code != 200) {
      return FALSE;
    }

    if (($index = strpos($result->data, 'contentUrl')) === FALSE) {
      return FALSE;
    }

    $content_data = substr($result->data, $index);
    if ($content_data) {
      $new_node = $space_node->createChild('cm_content', 'cm_contains', 'cm_'. $filename);
      $new_node->cm_name = $filename;
      $new_node->cm_content = $content_data;

      if (is_array($properties) && count($properties)) {
        foreach ($properties as $key => $value) {
          $new_node->$key = $value;
        }
      }

      $session->save();

      return TRUE;
    }
  }
  catch (SoapFault $exception) {
    alfresco_error_handler('alfresco', $exception);
  }
}

/**
 * Include Alfresco/Alfredo PHP files as necessary.
 */
function alfresco_service_include($file, $path = 'Alfresco/Service') {
  static $used = NULL;

  if (!isset($used)) {
    // Add the Alfresco PHP library directory to include path
    $lib_path = drupal_get_path('module', 'alfresco') .'/lib';
    if (is_dir($lib_path)) {
      set_include_path(get_include_path() . PATH_SEPARATOR . $lib_path);
    }
  }

  $filepath = $path .'/'. $file;
  if (!isset($used[$filepath])) {
    require_once "$filepath.php";
  }

  $used[$filepath] = TRUE;
}

/**
 * Authenticate against the repository and return a ticket.
 */
function _alfresco_service_authenticate($repository) {
  global $user;

  if (isset($_SESSION['alfresco']['ticket'])) {
    return $_SESSION['alfresco']['ticket'];
  }

  // The user credentials we are going to use
  $auth_method = variable_get('alfresco_authentication_method', ALFRESCO_AUTH_SINGLE);

  if ($user->uid == 0 || $user->uid == 1 && variable_get('alfresco_authentication_admin', TRUE) || $auth_method == ALFRESCO_AUTH_SINGLE) {
    $username = variable_get('alfresco_credentials_username', 'guest');
    $password = base64_decode(variable_get('alfresco_credentials_password', base64_encode('guest')));
  }
  else {
    $username = $_SESSION['alfresco']['credentials']['username'];
    $password = base64_decode($_SESSION['alfresco']['credentials']['password']);
  }

  $ticket = $repository->authenticate($username, $password);

  if ($ticket) {
    $_SESSION['alfresco']['ticket'] = $ticket;
  }

  return $ticket;
}
