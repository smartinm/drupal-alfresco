<?php
// $Id$

/**
 * @file
 * Alfresco Web Service.
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
  apm('[alfresco_service] Connecting to repository.');

  try {
    _alfresco_service_include_library();

    // The web service end point URL for the Alfresco repository
    $repository_url = variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api');

    // The user credentials we are going to use
    $username = variable_get('alfresco_credentials_username', 'guest');
    $password = base64_decode(variable_get('alfresco_credentials_password', base64_encode('guest')));
    //$password = variable_get('alfresco_credentials_password', 'guest');

    // Create the repository object baseed on the repository URL.  The repository represents the Alfresco repository we are connecting to.
    // It can be used to authenticate users and as a session factory.
    $repository = new Repository($repository_url);

    // Authenticate the user details for the repository we want to connect to.  Assuming authentication is succesful we will be returned
    // a ticket.  The ticket is a string which can be used to create Session's
    $ticket = @$repository->authenticate($username, $password);

    // Using the ticket we can now create a session.  In general it is better to create a new session for a new page or page sequence rather
    // than creating one and using across many pages as this helps to prevent resource usage from creaping up.
    $session = $repository->createSession($ticket);

    return $session;
  }
  catch (Exception $e) {
    drupal_set_message(t('Error connecting to alfresco repository.'), 'error');
    watchdog('alfresco', 'Error connecting to alfresco repository. URL=%url, Exception=%msg', array(
      '%url' => $repository_url,
      '%msg' => $e->getMessage()),
      WATCHDOG_ERROR);
  }
}

/**
 * Load a node by reference from alfresco repository.
 *
 * @param $reference
 *  The alfresco node reference.
 * @param $session Session
 * @return Node
 */
function alfresco_service_load_node($reference, $session = NULL) {
  apm('[alfresco_service] Loading node from repository.');

  if (!$inforef = alfresco_parse_reference($reference)) {
    return FALSE;
  }

  if (!$session && !($session = alfresco_service_create_session())) {
    return FALSE;
  }

  try {
    $store = $session->getStore($inforef['address'], $inforef['scheme']);

    // @see https://issues.alfresco.com/jira/browse/AR-1063
    $nodes = $session->query($store, '+ISNODE:T +ID:"'. $reference .'"');

    if ($count = count($nodes)) {
      if ($count > 1) {
        watchdog('alfresco', 'Se ha obtenido más de un nodo con la misma referencia %reference', array('%reference' => $reference), WATCHDOG_WARNING);
      }
      return $nodes[0];
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error loading node from alfresco repository.'), 'error');
    watchdog('alfresco', 'Error loading node from alfresco repository, with reference %reference. Exception=%msg', array(
      '%reference' => $reference,
      '%msg' => $e->getMessage()),
      WATCHDOG_ERROR);
  }
}

/**
 * Return an array of node properties from alfresco repository.
 *
 * @param $node Node Alfresco object node.
 */
function alfresco_service_load_properties($node) {
  apm('[alfresco_service] Updating properties from alfresco node.');

  $properties = array();

  try {
    $properties['cm_name']        = $node->cm_name;
    $properties['cm_title']       = $node->cm_title;
    $properties['cm_description'] = $node->cm_description;
    $properties['cm_author']      = $node->cm_author;
    $properties['cm_created']     = $node->cm_created;
    $properties['cm_creator']     = $node->cm_creator;
    $properties['cm_modified']    = $node->cm_modified;
    $properties['cm_modifier']    = $node->cm_modifier;
    $properties['cm_author']      = $node->cm_author;

    if (is_object($node->cm_content)) {
      $data = $node->cm_content;
      $properties['cm_content'] = array();
      $properties['cm_content']['url']      = $data->guestUrl;
      $properties['cm_content']['mimetype'] = $data->mimetype;
      $properties['cm_content']['size']     = $data->size;
      $properties['cm_content']['encoding'] = $data->encoding;
    }
    else {
      $properties['cm_content'] = NULL;
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error loading properties from alfresco node.'), 'error');
    watchdog('alfresco', 'Error loading properties from alfresco node, with reference %reference. Exception=%msg', array(
      '%reference' => strval($node),
      '%msg' => $e->getMessage()),
      WATCHDOG_ERROR);
  }

  return $properties;
}

/**
 * Include the relevant Alfresco PHP library and objets.
 */
function _alfresco_service_include_library() {
  static $included = FALSE;
  if ($included) {
    return;
  }

  // Include the required Alfresco PHP API objects
  @include_once "Alfresco/Service/Repository.php";
  @include_once "Alfresco/Service/Session.php";

  if (!class_exists('Repository')) {
    // Add the Alfresco PHP library directory to include path
    $lib_path = drupal_get_path('module', 'alfresco') .'/'. ALFRESCO_PHP_LIBRARY_PATH;
    if (is_dir($lib_path)) {
      set_include_path(get_include_path() . PATH_SEPARATOR . $lib_path);
    }

    // Include the required Alfresco PHP API objects
    @include_once "Alfresco/Service/Repository.php";
    @include_once "Alfresco/Service/Session.php";
  }

  $included = TRUE;
}
