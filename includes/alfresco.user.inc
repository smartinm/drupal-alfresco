<?php
// $Id$

/**
 * @file
 * Provides functions for xxx
 */

/**
 *
 */
function alfresco_user_login($edit) {
  // Cuando el usuario se autentica en Drupal guardamos las credenciales
  // para podernos autenticar contra Alfresco cuando sea necesario.
  // @todo: Analizar otros métodos, como guardar la sesión de alfresco
  $_SESSION['alfresco_credentials']['username'] = $edit['name'];
  $_SESSION['alfresco_credentials']['password'] = base64_encode($edit['pass']);

  // Eliminamos cualquier ticket anterior para forzar a realizar un nueva
  // autenticación con las credenciales del usuario.
  unset($_SESSION['alfresco_ticket']);
  unset($_SESSION['alfresco_last_access']);
}

/**
 *
 */
function alfresco_user_logout() {
  // No es necesario limpiar el ticket porque la sesión se destruye cuando un
  // usuario cierra la conexión.
  // @see user_logout()
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
