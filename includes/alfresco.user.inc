<?php
// $Id$

/**
 * @file alfresco.user.inc
 * Provides functions for xxx
 */

/**
 *
 */
function alfresco_user_login($edit) {
  // Cuando el usuario se autentica en Drupal guardamos las credenciales
  // para podernos autenticar contra Alfresco cuando sea necesario.
  // @todo: Analizar otros métodos, como guardar la sesión de alfresco
  $_SESSION['alfresco']['credentials']['username'] = $edit['name'];
  $_SESSION['alfresco']['credentials']['password'] = base64_encode($edit['pass']);
}

/**
 *
 */
function alfresco_user_logout() {
  // Limpiamos los datos de la sesión del usuario referente a Alfresco
  unset($_SESSION['alfresco']);
}

/**
 *
 */
function alfresco_user_view(&$account) {

}

/**
 *
 */
function alfresco_user_insert(&$edit, &$account) {

}

/**
 *
 */
function alfresco_user_update(&$edit, &$account) {

}

/**
 *
 */
function alfresco_user_delete($account) {

}
