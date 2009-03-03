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
    lines         : false,
    singleExpand  : false,
    trackMouseOver: false,
    useArrows     : true,

    loader : new Ext.tree.TreeLoader({
      dataUrl: 'alfresco/browser/json/spaces'
    }),

    root: new Ext.tree.AsyncTreeNode({id: 'NULL'}),
    
    tools:[{
      id: 'refresh',
      on: {
        click: function(){
          var tree = Ext.getCmp('tree-panel');
          tree.body.mask('Loading', 'x-mask-loading');
          tree.root.reload();
          setTimeout(function() {
            tree.body.unmask();
          }, 1000);
        }
      }
    }],
    
    listeners: {
      'click': function(node, e){
        store.load({params:{node:node.id}});
      }
    }
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
    url: 'alfresco/browser/json/items',
    root: 'nodes',
    remoteSort: false,
    fields: ['uuid', 'name', 'type', 'title', 'size', {name:'created', type:'date', dateFormat:'Y-m-d H:i:s'}, {name:'modified', type:'date', dateFormat:'Y-m-d H:i:s'}]
  });
  
  // Content Items Grid
  var grid = new Ext.grid.GridPanel( {
    store: store,
    columns: [{
      id: 'uuid',
      header: "UUID",
      sortable: false,
      dataIndex: 'uuid',
      hidden: true
    }, {
      id: 'name',
      header: "Name",
      renderer: function(value, p, record) {
        return '<span class="icon-' + record.data.type + '">' + value + '</span>';
      },
      sortable: true,
      dataIndex: 'name'
    }, {
      header: "Size",
      width: 75,
      sortable: true,
      dataIndex: 'size'
    }, {
      header: "Created",
      width: 125,
      sortable: true,
      renderer: Ext.util.Format.dateRenderer('Y-m-d H:i:s'),
      dataIndex: 'created'
    }, {
      header: "Modified",
      width: 125,
      sortable: true,
      renderer: Ext.util.Format.dateRenderer('Y-m-d H:i:s'),
      dataIndex: 'modified'
    }],
    
    listeners: {
      'rowdblclick': function(grid, rowIndex, e){
        
        var row = this.store.getAt(rowIndex);
        var uuid = row.get('uuid');
        if (row.get('type') == 'space') {
          store.load({params:{node:uuid}});
          //treePanel.getNodeById(uuid).getUI().ensureVisible();
          treePanel.getNodeById(uuid).expand();
        }
        else {
          
        }
      }
    },

    //stripeRows: false,
    autoExpandColumn :'name',
    trackMouseOver: false,
    loadMask: true
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
  store.load();
});
