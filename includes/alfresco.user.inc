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
  global $user;

  // Si el usuario no tiene habilitado acceso al repositorio de Alfresco,
  // utilizará las credenciales genéricas para descargar los elementos.
  if (isset($user->alfresco_access) && !$user->alfresco_access) {
    return;
  }

  // Autenticación contra Alfresco para validar la cuenta del usuario
  $auth_method = variable_get('alfresco_authentication_method', ALFRESCO_AUTH_SINGLE);
  if ($auth_method == ALFRESCO_AUTH_USER) {
    alfresco_include('soap');

    // Eliminamos la información de conexión como usuario anónimo, para forzar
    // a realizar la autenticación con las credenciales del usuario.
    alfresco_soap_clear_session();

    // Si el repositorio no está correctamente configurado no intentamos
    // validar la cuenta del usuario contra Alfresco.
    if (variable_get('alfresco_http_request_fails', FALSE)) {
      return;
    }

    $username = $edit['name'];
    $password = $edit['pass'];

    if (!alfresco_soap_verify_user($username, $password)) {
      // El proceso de autenticación ha fallado, marcamos el usuario como sin acceso.
      $_SESSION['alfresco_verified_account'] = FALSE;
    }
    else {
      // Marcamos el usuario como con acceso
      $_SESSION['alfresco_verified_account'] = TRUE;

      // Guardamos las credenciales para re-autenticar cuando el ticket caduque.
      $_SESSION['alfresco_credentials']['username'] = $username;
      $_SESSION['alfresco_credentials']['password'] = base64_encode($password);
    }
  }
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
function alfresco_user_insert(&$edit, $account) {

  // Default Alfresco user space.
  $edit['alfresco_home'] = '/app:company_home/app:user_homes/sys:'. $account->name;
}

/**
 *
 */
function alfresco_user_update($edit, $account) {

}

/**
 *
 */
function alfresco_user_delete($account) {

}

/**
 * The user account edit form is about to be displayed.
 */
function alfresco_user_form($edit, $account, $category) {

  $form['alfresco'] = array(
    '#type' => 'fieldset',
    '#title' => t('Alfresco settings'),
    //'#weight' => 5,
    '#collapsible' => TRUE,
  );

  $form['alfresco']['alfresco_access'] = array(
    '#type' => 'radios',
    '#title' => t('Repository access'),
    '#default_value' => isset($edit['alfresco_access']) ? $edit['alfresco_access'] : 1,
    '#options' => array(t('Disabled'), t('Enabled')),
    '#description' => t('Indica si el usuario tiene acceso directo al repositorio de Alfresco (para búscar, navegar, añadir contenido, etc).'),
  );
/*
  $form['alfresco']['alfresco_home'] = array(
    '#type' => 'textfield',
    '#title' => t('User home space'),
    '#default_value' => isset($edit['alfresco_home']) ? $edit['alfresco_home'] : '',
    '#description' => t('User home path on Alfresco repository.'),
    '#size' => 80,
  );
*/
  return $form;
}

/**
 * The user account is about to be modified.
 */
function alfresco_user_validate($edit, $account, $category) {
/*
  if ($edit['alfresco_access'] && !empty($edit['alfresco_home'])) {
    if (!alfresco_valid_path($edit['alfresco_home'])) {
      form_set_error('alfresco_home', t('The home path you specified is not valid.'));
    }
    else {
      alfresco_include('soap');
      if (!alfresco_soap_node_load($edit['alfresco_home'])) {
        form_set_error('alfresco_home', t('The home path you specified could not be found.'));
      }
    }
  }
*/
}
