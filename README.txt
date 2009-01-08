// $Id$

-- SUMMARY --

Alfresco is a related set of modules which provides Drupal integration with
Alfresco Open Source Enterprise Content Management System (ECM).

* alfresco.module: The main module. Provides a new 'Alfresco item' node content
  type for Drupal sites. The 'Alfresco item' node embeds Alfresco content
  inside your site, with support for direct and private download method of the
  files stored in Alfresco repository, caching of file properties, syncing of
  nodes, Views support, ... 

* alfresco_attach.module: Allows users to create and attach alfresco items to
  other Drupal content (similar to upload.module)

* alfresco_import.module: Allows importing multiple alfresco nodes from a space
  of Alfresco


-- REQUIREMENTS --

* Alfresco 3.x, 2.x (tested with Alfresco Labs and Alfresco Enterprise)

* Alfresco PHP Library (requires SOAP extension for PHP)

* PHP 5 (5.2.0 or greater)


-- INSTALLATION --

* Alfresco module requires the Alfresco PHP Library that allows remote access
  to the Alfresco repository via Web Services API. Alfresco module comes
  bundled the Alfresco PHP Library in the main module directory, you don't need
  install, only ensure that the SOAP extensions are installed and enabled in
  your PHP installation.
  
  Alternatively, if you want to install Alfresco PHP Library system-wide,
  follow these instructions:
   http://wiki.alfresco.com/wiki/Alfresco_PHP_Library_Installation_Instructions

* Install module as usual.


-- CONFIGURATION --

* Configure module settings in Administer >> Site configuration >> Alfresco:

  - Repository: Set URL and credentials for the Alfresco repository.
  
  - File downloads: Select the download method of the files stored in Alfresco
    repository. 
 
* Configure user permissions in Administer >> User management >> Permissions
  >> alfresco module.


-- CREDITS --

Author and maintainer:
 * Sergio Martín Morillas <smartin@gmv.com>
  
This project is sponsored by:
 * GMV <http://www.gmv.com>
