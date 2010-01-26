<?php
// $Id$

/**
 * @file alfresco.service.inc
 * Provides functions for access to Alfresco repository through of web service.
 */


/**
 * Authenticate against the repository and create a new session.
 *
 * @return Session
 *
 * @see http://wiki.alfresco.com/wiki/PHP_Tutorial_One
 * @see http://wiki.alfresco.com/wiki/PHP_Tutorial_One_Complete_Script
 */
function alfresco_service_create_session() {
  static $session = NULL;

  if (!isset($session)) {
    _alfresco_service_include_library();

    try {
      global $user;

      // The web service end point URL for the Alfresco repository
      $repository_url = variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api');

      // The user credentials we are going to use
      if ($user->uid == 0 || $user->uid == 1 && variable_get('alfresco_authentication_admin', TRUE)
          || variable_get('alfresco_authentication_method', ALFRESCO_AUTH_SINGLE) == ALFRESCO_AUTH_SINGLE) {
        $username = variable_get('alfresco_credentials_username', 'guest');
        $password = base64_decode(variable_get('alfresco_credentials_password', base64_encode('guest')));
      }
      else {
        $username = $_SESSION['alfresco']['credentials']['username'];
        $password = base64_decode($_SESSION['alfresco']['credentials']['password']);
      }

      // Create the repository object baseed on the repository URL.  The repository represents the Alfresco repository we are connecting to.
      // It can be used to authenticate users and as a session factory.
      $repository = new AlfRepository($repository_url);

      // Authenticate the user details for the repository we want to connect to.  Assuming authentication is succesful we will be returned
      // a ticket.  The ticket is a string which can be used to create Session's
      $ticket = $repository->authenticate($username, $password);

      // Using the ticket we can now create a session.  In general it is better to create a new session for a new page or page sequence rather
      // than creating one and using across many pages as this helps to prevent resource usage from creaping up.
      $session = $repository->createSession($ticket);
    }
    catch (Exception $e) {
      drupal_set_message(t('Error connecting to Alfresco repository. Please contact the server administrator for assistance.'), 'error');
      watchdog('alfresco', 'Error connecting to Alfresco repository. URL=%url, Exception=%msg', array(
        '%url' => $repository_url,
        '%msg' => _alfresco_service_error_message($e)),
        WATCHDOG_ERROR);
    }
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
function alfresco_service_node_load($reference_or_path, $session = NULL, $reset = NULL) {
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

  if (!$session && !($session = alfresco_service_create_session())) {
    return FALSE;
  }

  try {
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
  catch (Exception $e) {
    drupal_set_message(t('Error loading node from alfresco repository.'), 'error');
    watchdog('alfresco', 'Error loading node from alfresco repository, with reference %reference. Exception=%msg', array(
      '%reference' => $reference,
      '%msg' => _alfresco_service_error_message($e)),
      WATCHDOG_ERROR);
  }
}

/**
 *
 * @param $reference_or_path
 * @param $session
 * @param $reset
 * @return unknown_type
 */
function alfresco_service_node_delete($reference_or_path, $session = NULL) {

  if (!$inforef = alfresco_parse_reference($reference_or_path)) {
    if (!alfresco_valid_path($reference_or_path)) {
      return FALSE;
    }
  }

  if (!$session && !($session = alfresco_service_create_session())) {
    return FALSE;
  }

  try {
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
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error loading node from alfresco repository.'), 'error');
    watchdog('alfresco', 'Error loading node from alfresco repository, with reference %reference. Exception=%msg', array(
      '%reference' => $reference,
      '%msg' => _alfresco_service_error_message($e)),
      WATCHDOG_ERROR);
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
function alfresco_service_search($text, $property = '@cm:name', $session = NULL) {

  if (!$session && !($session = alfresco_service_create_session())) {
    return FALSE;
  }

  try {
    // Lucene requires the : to be escaped using the \ character
    $property = !empty($property) ? str_replace(':', '\:', $property) : 'ALL';
    $query = '+ISNODE:T +TYPE:"cm:content" +'. $property .':"*'. $text .'*"';

    $spacesStore = new AlfSpacesStore($session);
    $queryResults = $session->query($spacesStore, $query);

    //$nodes = $session->query($store, 'PATH:"/app:company_home/*"');

    return $queryResults;
  }
  catch (Exception $e) {
    drupal_set_message(t('Error searching nodes from alfresco repository.'), 'error');
    watchdog('alfresco', 'Error searching node from alfresco repository, with reference %reference. Exception=%msg', array(
      '%reference' => $reference,
      '%msg' => _alfresco_service_error_message($e)),
      WATCHDOG_ERROR);
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
function alfresco_service_add_content($space, $filepath, $filename = NULL, $mimetype = NULL, $encoding = 'UTF-8', $properties = array(), $session = NULL) {

  if (!$session && !($session = alfresco_service_create_session())) {
    return FALSE;
  }

  try {

    $space_node = alfresco_service_node_load($space, $session);
    if (!$space_node) {
      return FALSE;
    }

    // Get the address and the
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
  catch (Exception $e) {
    drupal_set_message(t('Error saving alfresco node.'), 'error');
    watchdog('alfresco', 'Error saving alfresco node. Exception=%msg', array(
      '%msg' => _alfresco_service_error_message($e)),
      WATCHDOG_ERROR);
  }
}

/**
 * Include the Alfresco PHP library.
 */
function _alfresco_service_include_library() {
  static $included = FALSE;

  if (!$included) {
    // Add the Alfresco PHP library directory to include path
    $lib_path = drupal_get_path('module', 'alfresco') .'/lib';
    if (is_dir($lib_path)) {
      set_include_path(get_include_path() . PATH_SEPARATOR . $lib_path);
    }

    // Include the required Alfresco PHP API objects
    include_once 'Alfresco/Service/SpacesStore.php';
    include_once 'Alfresco/Service/Repository.php';
    include_once 'Alfresco/Service/Session.php';

    $included = TRUE;
  }
}

/**
 *
 */
function _alfresco_service_error_message(Exception $ex) {

  if (empty($ex)) {
    return NULL;
  }

  if ($ex instanceof SoapFault) {
    $error = alfresco_parse_soap_fault($ex);
    $message = t('SOAP Fault Code: !code, Message: !message, Detail: !detail', array(
      '!code'    => !empty($error['code'])    ? $error['code']    : t('None'),
      '!message' => !empty($error['message']) ? $error['message'] : t('None'),
      '!detail'  => !empty($error['detail'])  ? $error['detail']  : t('None'),
    ));
  }
  else {
    $message = $ex->getMessage();
  }

  return $message;
}

function alfresco_service_node_content($node) {

  if (isset($node) && is_object($node->cm_content)) {
    if ($node->cm_content instanceof AlfContentData) {
      return $node->cm_content;
    }
  }
  return FALSE;
}
