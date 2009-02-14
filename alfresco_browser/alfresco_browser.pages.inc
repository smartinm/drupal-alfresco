<?php
// $Id$

/**
 * @file
 *
 */

/**
 *
 */
function alfresco_browser_page() {
  print theme('alfresco_browser_page');
}

function alfresco_browser_json() {
  $count1 = rand(); $count2 = rand(); $count3 = rand();
  $icon_path = drupal_get_path('module', 'alfresco_browser') . '/images/space-icon-default-16.gif';

  print "[{
        id: $count1,
        text: 'A leaf Node',
        icon: '$icon_path',
        leaf: false
    },{
        id: $count2,
        text: 'A folder Node',
        icon: '$icon_path',
        children: [{
            id: $count3,
            text: 'A child Node',
            icon: '$icon_path',
            leaf: true
        }]
   }]";
}
