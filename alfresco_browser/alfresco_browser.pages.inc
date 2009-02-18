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

function alfresco_browser_json($action) {

  if ($action == 'xxx') {
    echo "
{
    nodes: [
        {name: 'd72_2003.pdf', description:'Decreto 72/2003, de 18 de marzo', size:46.5, created: new Date(2007, 10, 29), modified: new Date(2007, 10, 29)},
        {name: 'decreto104-92.pdf', description:'Decreto 104/1992, de 9 de junio', size:46.5, created: new Date(2007, 10, 29), modified: new Date(2007, 10, 29)},
        {name: 'd72_2003.pdf', description:'Medidas de Impulso de la Sociedad del Conocimiento en Andalucía', size:46.5, created: new Date(2007, 10, 29), modified: new Date(2007, 10, 29)},
        {name: 'decreto104-92.pdf', description:'Decreto 104/1992', size:46.5, created: new Date(2007, 10, 29), modified: new Date(2007, 10, 29)},
        {name: 'd72_2003.pdf', description:'Decreto 72/2003, de 18 de marzo, de Medidas de Impulso de la Sociedad del Conocimiento en Andalucía', size:46.5, created: new Date(2007, 10, 29), modified: new Date(2007, 10, 29)},
        {name: 'decreto104-92.pdf', description:'elaboración y desarrollo de la Política Informática de la Junta de Andalucía', size:46.5, created: new Date(2007, 10, 29), modified: new Date(2007, 10, 29)}
    ]
}
    ";
    exit();
  }

  $count1 = rand(); $count2 = rand(); $count3 = rand();
  $icon_path = drupal_get_path('module', 'alfresco_browser') . '/images/space-icon-default-16.gif';

  $spaces = array();
  $spaces[] = array(
    'id' => $count1,
    'text' => 'Proyectos',
    'icon' => $icon_path,
    'leaf' => TRUE,
  );
  $spaces[] = array(
    'id' => $count2,
    'text' => 'OVCTIC',
    'icon' => $icon_path,
  );
  drupal_json($spaces);
  exit();

  print "[{
        id: $count1,
        text: 'Proyectos',
        icon: '$icon_path',
        leaf: false
    },{
        id: $count2,
        text: 'OVCTIC',
        icon: '$icon_path',
        children: [{
            id: $count3,
            text: 'Normativas',
            icon: '$icon_path',
            leaf: true
        }]
   }]";
}
