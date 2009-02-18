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
      dataUrl : 'alfresco/browser/actions/loadSpaces'
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
  
  // create the Data Store
  var store = new Ext.data.JsonStore({
    url: 'alfresco/browser/actions/xxx',
    root: 'nodes',
    remoteSort: true,
    fields: ['name', 'description', {name:'size', type: 'float'}, {name:'created', type:'date'}, {name:'modified', type:'date'}]
  });
  
  var pagingBar = new Ext.PagingToolbar({
    pageSize: 25,
    store: store,
    displayInfo: true,
    displayMsg: 'Displaying topics {0} - {1} of {2}',
    emptyMsg: "No topics to display",
  });

  // Content Items Grid
  var grid = new Ext.grid.GridPanel( {
    store: store,
    columns: [{
      id: 'name',
      header: "Name",
      width: 160,
      renderer: function(val) {
        return '<span class="icon-test">' + val + '</span>';
      },
      sortable: true,
      dataIndex: 'name'
    }, {
      id: 'description',
      header: "Title",
      width: 75,
      sortable: false,
      dataIndex: 'description'
    }, {
      header: "Size",
      width: 75,
      sortable: true,
      dataIndex: 'size'
    }, {
      header: "Created",
      width: 75,
      sortable: true,
      renderer: Ext.util.Format.dateRenderer('m/d/Y'),
      dataIndex: 'created'
    }, {
      header: "Modified",
      width: 75,
      sortable: true,
      renderer: Ext.util.Format.dateRenderer('m/d/Y'),
      dataIndex: 'modified'
    }],
    stripeRows :true,
    autoExpandColumn :'description',
    //trackMouseOver: false,
    loadMask: true,
    bbar: pagingBar
    //height :350,
    //width :600,
    //title :'Array Grid'
  });

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
      border :false,
      //bodyStyle :'padding:25px',
      items : grid
    }],
    renderTo : Ext.getBody()
  });
  
  // trigger the data store load
  store.load({params:{start:0, limit:25}});
});
