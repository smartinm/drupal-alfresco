// $Id$

//
// Main layout definition.
//
Ext.onReady(function() {

  //Ext.state.Manager.setProvider(new Ext.state.CookieProvider());

  Ext.QuickTips.init();

  var treePanel = new Ext.tree.TreePanel({
    id            : 'tree-panel',
    title         : 'Browse Spaces',
    region        : 'north',
    split         : true,
    height        : 300,
    minSize       : 150,
    autoScroll    : true,

    // tree-specific configs:
    rootVisible   : false,
    //lines         : false,
    //singleExpand  : true,
    useArrows     : true,

    loader        : new Ext.tree.TreeLoader({
      dataUrl : 'alfresco/browser/dummy'
    }),

    root          : new Ext.tree.AsyncTreeNode({
      id:'xxx'
    })
  });

  var detailsPanel = {
    id :'details-panel',
    title :'Details',
    region :'center',
    bodyStyle :'padding-bottom:15px;background:#eee;',
    autoScroll :true,
    html :'<p class="details-info">Información del fichero</p>'
  };

  new Ext.Viewport({
    layout : 'border',
    items  : [{
      //xtype :'box',
      region :'north',
      height : 35,
      //autoEl: {tag:'div', id:'header', html:'<h1>Alfresco Browser</h1>'}
      html : '<div id="header"><h1>Alfresco Node Browser for Drupal</h1></div>'
    }, {
      layout :'border',
      id :'layout-browser',
      region :'west',
      border :false,
      split :true,
      margins :'2 0 5 5',
      width :275,
      minSize :100,
      maxSize :500,
      items : [ treePanel, detailsPanel ]
    }, {
      id :'content-panel',
      title :'Content Items',
      region :'center', // this is what makes this panel into a region within
      layout :'fit',
      margins :'2 5 5 0',
      //border :false,
      bodyStyle :'padding:25px',
      bbar: new Ext.StatusBar({
        defaultText   : 'Default status',
        id            : 'basic-statusbar',
        items         : [{
          text: 'A Button'
        }, '-', 'Plain Text', ' ', ' ']
      })
    }],
    renderTo : Ext.getBody()
  });
});
