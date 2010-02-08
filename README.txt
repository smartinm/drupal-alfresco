// $Id$

-- SUMMARY --

Alfresco module es una solución out of the box para integrar un sitio Drupal
con Alfresco Open Source Open Source Enterprise Content Management System (ECM).
Está formado por un conjunto de módulos que ofrecen múltiples funcionalidades
listas para usar con mínima configuración. El núcleo de la integración está
basado en el tipo de contenido 'Alfresco Item', siguiendo el paradigma de
elementos de Alfresco como nodos de Drupal. 

* alfresco.module: Es el módulo principal. Ofrece un nuevo tipo de contenido
  'Elemento de Alfresco' para Drupal. Los nodos de tipo 'Elemento de Alfresco'
  permiten embeber contenido de Alfresco dentro de tu sitio. Además, este módulo
  ofrece la configuración y API necesaria para realizar la integración con el
  repositorio de Alfresco: descargas de ficheros, obtención de metadatos,
  autenticación, Views support, etc.

* alfresco_browser.module: Permite navegar y buscar por el repositorio de
  Alfresco, además de descargar, ver y crear nuevos contenidos en el mismo. 

Alfresco project is a related set of modules which provides Drupal integration
with Alfresco Open Source Enterprise Content Management System (ECM).

* alfresco.module: The main module. Provides a new 'Alfresco item' node content
  type for Drupal sites. The 'Alfresco item' node embeds Alfresco contents
  inside your site, with support for direct and private download of the files
  stored in Alfresco repository, caching of file properties, syncing of nodes,
  Views support, ... 

* alfresco_browser.module: Allows users to visualize, search, browse and
  retrieve nodes of the Alfresco repository.

* alfresco_attach.module: Allows users to create and attach alfresco items to
  other Drupal content (similar to upload.module).

* alfresco_import.module: Allows importing multiple alfresco nodes from a space
  of Alfresco.


-- REQUIREMENTS --

* Alfresco 3.x or 2.x (Alfresco Community or Alfresco Enterprise)

* PHP 5.2 or later, with
  * DOM Extension (part of PHP 5 core)
  * SOAP Extension (--enable-soap)

* Ext JS - JavaScript Library
  * Only for Alfresco browser module


-- INSTALLATION --

* Note that you will need an installed and configured Alfresco (on remote or
  local server) prior to installing this module.

* Install Alfresco module as usual on your Drupal site.


-- CONFIGURATION --

* Configure module settings in Administer >> Site configuration >> Alfresco:

  - Repository: Set URL and credentials for the Alfresco repository.
  
  - File downloads: Select the download method of the files stored in Alfresco
    repository. 
 
* Configure user permissions in Administer >> User management >> Permissions
  >> alfresco module


-- CREDITS --

Author and maintainer:
* Sergio Martín Morillas <smartin@gmv.com> - http://drupal.org/user/191570

Contributors:
* Manuel Jesús Recena Soto
  Thanks for your helpful comments and encouragement.

This project is sponsored by:
* GMV
  Technological business group with an international presence. GMV offers its
  solutions, services and products in very diverse sectors: Aeronautics,
  Banking and Finances, Space, Defense, Health, Security, Transportation,
  Telecommunications, and Information Technology for Public Administration and
  large corporations. Visit http://www.gmv.com for more information. 

* Some of the icons used by this module are part of the Alfresco Community
  Edition.
