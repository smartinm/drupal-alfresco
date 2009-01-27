// $Id$

-- SUMMARY --

Alfresco project is a related set of modules which provides Drupal integration
with Alfresco Open Source Enterprise Content Management System (ECM).

* alfresco.module: The main module. Provides a new 'Alfresco item' node content
  type for Drupal sites. The 'Alfresco item' node embeds Alfresco contents
  inside your site, with support for direct and private download of the files
  stored in Alfresco repository, caching of file properties, syncing of nodes,
  Views support, ... 

* alfresco_attach.module: Allows users to create and attach alfresco items to
  other Drupal content (similar to upload.module).

* alfresco_import.module: Allows importing multiple alfresco nodes from a space
  of Alfresco.


-- REQUIREMENTS --

* Alfresco 3.x or 2.x (tested with Alfresco Labs and Alfresco Enterprise)

* Alfresco PHP Library (included in Alfresco module bundle) 

* PHP 5 (5.2.0 or greater) with SOAP extension enabled


-- INSTALLATION --

* Note that you will need an installed and configured Alfresco prior to
  installing this module.

* Alfresco module requires the Alfresco PHP Library that allows remote access
  to the Alfresco repository via Web Services API. Alfresco module comes
  bundled the Alfresco PHP Library in the main module directory, so you don't
  need install anything. Simply ensure that the SOAP extensions are installed
  and enabled in your PHP installation:
    http://php.net/manual/soap.installation.php
  
  Optionally, if you want to install Alfresco PHP Library system-wide, you can
  follow these instructions:
    http://wiki.alfresco.com/wiki/Alfresco_PHP_Library_Installation_Instructions

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
* Manuel Jesús Recena Soto <mjrecena@gmv.com>
  Thanks for your helpful comments and encouragement.

This project is sponsored by:
* GMV
  Technological business group with an international presence. GMV offers its
  solutions, services and products in very diverse sectors: Aeronautics,
  Banking and Finances, Space, Defense, Health, Security, Transportation,
  Telecommunications, and Information Technology for Public Administration and
  large corporations. Visit http://www.gmv.com for more information. 
