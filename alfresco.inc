<?php
/*
 * OFICINA VIRTUAL DE COORDINACIÓN DE LAS TIC
 *
 * Empresa desarrolladora:
 *     GMV Soluciones Globales Internet, S.A.
 *     Avda. Américo Vespucio, Nº5
 *     Edificio Cartuja, Bloque E, 1ª Planta
 *     41092 Sevilla (España)
 *
 * Autor: Junta de Andalucía. Derechos de explotación
 *     propiedad de la Junta de Andalucía
 *
 * Este programa es software libre: usted tiene derecho a
 * redistribuirlo y/o modificarlo bajo los términos de la
 * Licencia EUPL European Public License publicada por el
 * organismo IDABC de la Comisión Europea, en su versión 1.0.
 * o posteriores.
 *
 * Este programa se distribuye de buena fe, pero SIN NINGUNA
 * GARANTÍA, incluso sin las presuntas garantías implítareas
 * de USABILIDAD o ADECUACIÓN A PROPÓSITO CONCRETO. Para mas
 * información consulte la Licencia EUPL European Public
 * License.
 *
 * Usted recibe una copia de la Licencia EUPL European Public
 * License junto con este programa, si por algún motivo no le
 * es posible visualizarla, puede consultarla en la siguiente
 * URL: http://ec.europa.eu/idabc/servlets/Doc?id=27470
 *
 * You should have received a copy of the EUPL European Public
 * License along with this program. If not, see
 * http://ec.europa.eu/idabc/servlets/Doc?id=27470
 *
 * Vous devez avoir reçu une copie de la EUPL European Public
 * License avec ce programme. Si non, voir
 * http://ec.europa.eu/idabc/servlets/Doc?id=27470
 *
 * Sie sollten eine Kopie der EUPL European Public License
 * zusammen mit diesem Programm. Wenn nicht, finden Sie da
 * http://ec.europa.eu/idabc/servlets/Doc?id=27470
 *
 * $Id$
 */
 
/**
 * @file
 * Formulario de edición.
 */


/**
 * Implementation of hook_form().
 *
 * Now it's time to describe the form for collecting the information
 * specific to this node type. This hook requires us to return an array with
 * a sub array containing information for each element in the form.
 */
function alfresco_form(&$node, $form_state) {

  // The site admin can decide if this node type has a title and body, and how
  // the fields should be labeled. We need to load these settings so we can
  // build the node form correctly.
  $type = node_get_types('type', $node);

  if ($type->has_title) {
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => check_plain($type->title_label),
      '#required' => FALSE,
      '#default_value' => $node->title,
      '#weight' => -5
    );
  }

  if ($type->has_body) {
    // In Drupal 6, we can use node_body_field() to get the body and filter
    // elements. This replaces the old textarea + filter_form() method of
    // setting this up. It will also ensure the teaser splitter gets set up
    // properly.
    $form['body_field'] = node_body_field($node, $type->body_label, $type->min_word_count);
  }

  $form['reference'] = array(
    '#type' => 'textfield',
    '#title' => t('Node Reference'),
    '#required' => TRUE,
    '#default_value' => $node->reference,
    '#weight' => -10,
    '#description' => t('The Alfresco Node Reference (workspace://SpacesStore/ID).'),
  );

  return $form;
}

/**
 * Implementation of hook_view().
 *
 * This is a typical implementation that simply runs the node text through
 * the output filters.
 */
function alfresco_view($node, $teaser = FALSE, $page = FALSE) {
  $node = node_prepare($node, $teaser);
  
  // Include the required Alfresco PHP API objects  
  require_once "Alfresco/Service/Repository.php";
  require_once "Alfresco/Service/Session.php";
  require_once "Alfresco/Service/SpacesStore.php";

  // The web service end point URL for the Alfresco repository
  $repositoryUrl = "http://localhost:8080/alfresco/api";
  
  // The user credentials we are going to use
  $userName = "admin";
  $password = "admin"; 
  
  // Create the repository object baseed on the repository URL.  The repository represents the Alfresco repository we are connecting to.
  // It can be used to authenticate users and as a session factory.
  $repository = new Repository($repositoryUrl);
  
  // Authenticate the user details for the repository we want to connect to.  Assuming authentication is succesful we will be returned
  // a ticket.  The ticket is a string which can be used to create Session's
  $ticket = $repository->authenticate($userName, $password);
  
  // Using the ticket we can now create a session.  In general it is better to create a new session for a new page or page sequence rather 
  // than creating one and using across many pages as this helps to prevent resource usage from creaping up.
  $session = $repository->createSession($ticket);

  // Create a reference to the 'SpacesStore'
  $spacesStore = new SpacesStore($session);
  
  $currentNode = $session->getNode($spacesStore, 'c55a02a0-bbc3-11dd-b785-274d22f26a6c');
  
  $properties = array();
  foreach ($currentNode->properties as $name=>$value) {
    if(is_string($value)) {
      $properties[] = array($name, $value);
    }
    else {
      $properties[] = array($name, $value->mimetype);
    }
  }
  
  $contentData = $currentNode->cm_content;
  
  $node->content['alfresco'] = array(
    '#value' => theme('table', array('Propiedad', 'Valor'), $properties) . l('Descarga', $contentData->getUrl()),
    '#weight' => 1,
  );
  
  /*
  if (ini_get('allow_url_fopen') == '1') {
    dpm("allow_url_fopen");
    $url = $contentData->getUrl(); 
    // make sure the remote file is successfully opened before doing anything else
    if ($fp = fopen($url, 'r')) {
      dpm("fopen");
       $content = '';
       // keep reading until there's nothing left
       while ($line = fread($fp, 1024)) {
          $content .= $line;
       }
    
       print $content;
    } else {
      dpm("ERROR");
       // an error occured when trying to open the specified url
    }
  } 
  */

  return $node;
}
