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
  apm('alfresco_service_create_session');
  
  try {
    _alfresco_service_include_library();
    
    // The web service end point URL for the Alfresco repository
    $repositoryUrl = variable_get('alfresco_repository_url', 'http://localhost:8080/alfresco/api');
    
    // The user credentials we are going to use
    $username = variable_get('alfresco_credentials_username', 'guest');
    $password = base64_decode(variable_get('alfresco_credentials_password', base64_encode('guest')));
    //$password = variable_get('alfresco_credentials_password', 'guest');
    
    // Create the repository object baseed on the repository URL.  The repository represents the Alfresco repository we are connecting to.
    // It can be used to authenticate users and as a session factory.
    $repository = new Repository($repositoryUrl);
    
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
      '%url' => $repositoryUrl,
      '%msg' => $e->getMessage()),
      WATCHDOG_ERROR);
  }
}

/**
 * Load a node by reference from alfresco repository.
 * 
 * @param $reference Alfresco Node Reference.
 * @param $session Session
 * @return Node
 */
function alfresco_service_load_node($reference, $session = NULL) {
  apm('alfresco_service_load_node');
  
  if (!$inforef = alfresco_parse_reference($reference)) {
    return FALSE;
  }
  
  if (!$session && !($session = alfresco_service_create_session())) {
    return FALSE;
  }
  
  try {
    $store = $session->getStore($inforef['address'], $inforef['scheme']);
    $nodes = $session->query($store, 'ID:"'. $reference .'"');
    
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
      '%reference' => $repositoryUrl,
      '%msg' => $e->getMessage()),
      WATCHDOG_ERROR);
  }
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
    $lib_path = drupal_get_path('module', 'alfresco') . '/'. ALFRESCO_PHP_LIBRARY_PATH;
    if (is_dir($lib_path)) {
      set_include_path(get_include_path() . PATH_SEPARATOR . $lib_path);
    }
    
    // Include the required Alfresco PHP API objects  
    @include_once "Alfresco/Service/Repository.php";
    @include_once "Alfresco/Service/Session.php";
  }
  
  $included = TRUE;
}
