// $Id$

/*
 * Ext JS Alfresco Browser for Drupal.
 * 
 * Module Pattern
 * http://extjs.com/learn/Manual:Basic_Application_Design
 */
Ext.ns('AlfrescoBrowser');

AlfrescoBrowser.ViewItem = function (url, title) {
  var size = Ext.getCmp('alfresco-browser-viewport').getSize();
  var iframeWin = new Ext.Window({
    id: 'preview-window',
    title: title,
    width: size.width - 100,
    height: size.height - 100,
    maximizable: true,
    modal: 'true',
    layout: 'fit', 
    html: '<iframe id="preview-frame" frameborder="0"  src="' + url + '" onLoad="AlfrescoBrowser.ViewItemOnLoad()"></iframe>',
    buttonAlign: 'center',
    defaultButton: 0,
    buttons: [{
      text: 'Close',
      handler: function() {
        iframeWin.close();
      }
    }]
  });
  iframeWin.show();
  
  var mask = new Ext.LoadMask(iframeWin.body);
  mask.show();
}

AlfrescoBrowser.ViewItemOnLoad = function () {
  var iframeWin = Ext.getCmp('preview-window');
  if (iframeWin) {
    iframeWin.body.unmask();
  }
}

AlfrescoBrowser.SendToDrupal = function (node) {
  var title = Ext.util.Format.trim(node.title);
  var reference = 'workspace://SpacesStore/' + node.id;
  
  window.opener.$("#edit-alfresco-browser-reference").val(reference);
  window.opener.$("#edit-alfresco-browser-path").val(node.name);
  
  if (window.opener.$("#alfresco-edit-title-wrapper #edit-title").length > 0) {
    window.opener.$("#alfresco-edit-title-wrapper #edit-title").val(title);
  }
  
  window.opener.focus();
  self.close();
}

AlfrescoBrowser.UploadItem = function (space) {

  var uploadForm = new Ext.FormPanel({
    fileUpload: true,
    width: 500,
    frame: true,
    autoHeight: true,
    bodyStyle: 'padding: 10px 10px 0 10px;',
    labelWidth: 50,
    defaults: {
        anchor: '95%',
        allowBlank: false,
        msgTarget: 'side'
    },
    items: [{
        xtype: 'fileuploadfield',
        id: 'form-file',
        name: 'files[file]',
        emptyText: 'Locate content to upload.',
        fieldLabel: 'File',
        listeners: {
          'fileselected': function(fb, v) {
            Ext.getCmp('form-name').setValue(v);
          }
        }
    }, {
      xtype: 'textfield',
      id: 'form-name',
      name: 'name',
      fieldLabel: 'Name'
    }, {
      xtype: 'hidden',
      name: 'space',
      value: space
    }],
    buttons: [{
        text: 'Add',
        handler: function() {
          if (uploadForm.getForm().isValid()) {
            uploadForm.getForm().submit({
              url: AlfrescoBrowser.Settings['serviceUploadUrl'],
              waitMsg: 'Uploading your content...',
              success: function(uploadForm, o) {
                dataStore.reload();
              }
            });
          }
        }
    }, {
        text: 'Reset',
        handler: function(){
          uploadForm.getForm().reset();
        }
    }]
  });

  var win = new Ext.Window({
    id: 'upload-window',
    title: 'Add content to current space',
    autoHeight: true,
    minWidth: 500,
    modal: 'true',
    layout: 'fit', 
    items: [ uploadForm ]
  });
  
  win.show();
}

AlfrescoBrowser.App = function() {
  var folderTree;
  var itemsGrid;
  var propsGrid;
  var dataStore;
  
  return {
    init: function() {
      //Ext.state.Manager.setProvider(new Ext.state.CookieProvider());
      
      Ext.QuickTips.init();
      
      this.initFolderTree();
      this.initDocumentGrid();
      this.initSearch();
      this.initLayout();
      
      /*
      folderTree.expandPath('/1b6feb86-bc5e-11dd-8590-695888b0c20c/1df6475a-bc5e-11dd-8590-695888b0c20c/aecb9b3c-e3ce-11dd-a77e-2daab57b0158');
      folderTree.selectPath('/1b6feb86-bc5e-11dd-8590-695888b0c20c/1df6475a-bc5e-11dd-8590-695888b0c20c/aecb9b3c-e3ce-11dd-a77e-2daab57b0158');
      
      var node = folderTree.getSelectionModel().getSelectedNode();
      //node.fireEvent('click', node);
      */
    },
    initLayout: function(){
      // --------------------------------------------
      // -- LAYOUT
      // --------------------------------------------
      var viewport = new Ext.Viewport({
        id: 'alfresco-browser-viewport',
        layout: 'border',
        items: [new Ext.BoxComponent({
          region: 'north',
          el: 'header',
          height: 35
        }), folderTree, {
          region: 'center',
          layout: 'border',
          border: false,
          margins: '5 5 5 0',
          items: [ itemsGrid, propsGrid ]
        }]
      });
    },
    initFolderTree: function(){
      // --------------------------------------------
      // -- NAVIGATION TREE
      // --------------------------------------------
      folderTree = new Ext.tree.TreePanel({
        id: 'folder-tree',
        region: 'west',
        collapsible: true,
        title: 'Browse Spaces',
        margins: '5 0 5 5',
        cmargins: '5 5 5 5',
        width: 240,
        minSize: 150,
        maxSize: 400,
        split: true,
        layout: 'fit',
        autoScroll: true,
        
        // tree specifics
        rootVisible: true,
        useArrows: true,
        trackMouseOver: false,

        loader: new Ext.tree.TreeLoader({
          dataUrl: AlfrescoBrowser.Settings['serviceTreeUrl'],
          requestMethod: 'GET'
        }),
        
        root: new Ext.tree.AsyncTreeNode({
          text: AlfrescoBrowser.Settings['homeText'],
          id: AlfrescoBrowser.Settings['homeRef'],
          expanded: true
        }),
        
        listeners: {
          'click': function(node, e) {
            node.expand();
            dataStore.baseParams = {node: node.id};
            dataStore.load({params:{start:0}});
            itemsGrid.setTitle(node.text);
        }},
        
        tools: [{
          id: 'refresh',
          on: {
            click: function(){
              var currentNode = folderTree.getSelectionModel().getSelectedNode();
              if (Ext.isEmpty(currentNode)) {
                currentNode = folderTree.root;
              }
              var currentPath = currentNode.getPath();
              
              var baseParams = folderTree.loader.baseParams || {};
              folderTree.loader.baseParams['cache'] = 'all';
              folderTree.root.reload();
              folderTree.loader.baseParams = baseParams;
              
              folderTree.expandPath(currentPath);
              folderTree.selectPath(currentPath);
            }}
        }]
      });
    },
    initSearch: function(){
      // --------------------------------------------
      // -- SEARCH
      // --------------------------------------------
      Ext.app.SearchField = Ext.extend(Ext.form.TwinTriggerField, {
        initComponent : function(){
            Ext.app.SearchField.superclass.initComponent.call(this);
            this.on('specialkey', function(f, e){
                if(e.getKey() == e.ENTER){
                    this.onTrigger2Click();
                }
            }, this);
        },

        validationEvent:false,
        validateOnBlur:false,
        trigger1Class:'x-form-clear-trigger',
        trigger2Class:'x-form-search-trigger',
        hideTrigger1: true,
        hasSearch: false,
        paramName: 'query',
        
        onTrigger1Click : function(){
            if(this.hasSearch){
                this.el.dom.value = '';
                var node = folderTree.getSelectionModel().getSelectedNode();
                if (Ext.isEmpty(node)) {
                  node = folderTree.root;
                }
                this.store.baseParams = {node:node.id};
                this.store.load({params:{start:0}});
                this.triggers[0].hide();
                this.hasSearch = false;
                itemsGrid.setTitle(node.text);
            }
        },

        onTrigger2Click : function(){
            var v = this.getRawValue();
            if (v.length < 1) {
              this.onTrigger1Click();
              return;
            }
            if (v.length < 3) {
              return;
            }
            itemsGrid.setTitle('Search items: ' + Ext.util.Format.htmlEncode(v));

            this.store.baseParams[this.paramName] = v;
            this.store.load({params:{start:0}});
            this.hasSearch = true;
            this.triggers[0].show();
        }
      });

      var search = new Ext.app.SearchField({
        region: 'west',
        store: dataStore,
        width: 320,
        emptyText: 'Search (minimum 3 characters)',
        applyTo: 'search'
      })
    },
    initDocumentGrid: function(){
      // --------------------------------------------
      // -- CONTENT GRID
      // --------------------------------------------
      var record = Ext.data.Record.create([
        {name: 'id'},   /* alfresco node id */
        {name: 'nid'},  /* drupal node id */
        {name: 'type'},
        {name: 'name'},
        {name: 'icon'},
        {name: 'author'},
        {name: 'creator'},
        {name: 'title'},
        {name: 'description'},
        {name: 'size'},
        {name: 'mimetype'},
        {name: 'created', type: 'date', dateFormat: 'Y-m-d H:i:s'},
        {name: 'modified', type: 'date', dateFormat: 'Y-m-d H:i:s'}
      ]);
      
      var reader = new Ext.data.JsonReader({
          totalProperty: 'total',
          root: 'rows',
          id: 'id'
      }, record);
      
      dataStore = new Ext.data.Store({
        proxy: new Ext.data.HttpProxy({
          url: AlfrescoBrowser.Settings['serviceGridUrl'],
          method: 'GET'
        }),
        reader: reader,
        baseParams: {node: AlfrescoBrowser.Settings['homeRef']},
        autoLoad: true,
        //remoteSort: true,
        sortInfo: {field: 'name', direction: 'ASC'}
      });
      
      function renderName(value, p, record){
        var url = AlfrescoBrowser.Settings['modulePath'] + '/images/filetypes/' + record.data['icon'] + '.gif';
        return String.format('<span class="row-icon" style="background-image: url({1})" ext:qtip="{2}">{0}</span>', value, url, record.data['title']);
      }

      var columns = [
        {id: 'name', header: "Name", dataIndex: 'name', sortable: true, width: 200, renderer: renderName},
        {header: "Size", dataIndex: 'size', sortable: false, align: 'right', width: 80},
        {header: "Creator", dataIndex: 'creator', sortable: true, width: 100},
        {header: "Date created", dataIndex: 'created', width: 130, hidden: true, sortable: true, renderer: Ext.util.Format.dateRenderer('d-m-Y H:i:s')},
        {header: "Date modified", dataIndex: 'modified', width: 130, sortable: true, renderer: Ext.util.Format.dateRenderer('d-m-Y H:i:s')},
        {header: "Title", dataIndex: 'title', width: 200, sortable: true, hidden: true},
        {header: "Description", dataIndex: 'description', width: 200, sortable: false, hidden: true},
        {header: "Author", dataIndex: 'author', width: 100, sortable: true, hidden: true}
      ];
      
      var bar = new Ext.PagingToolbar({
        pageSize: AlfrescoBrowser.Settings['queryLimit'],
        store: dataStore,
        displayInfo: true,
        autoWidth: true,
        displayMsg: 'Displaying items {0} - {1} of {2}',
        emptyMsg: 'No items to display.',

        // override private event
        onClick: function(which){
          if (which == "refresh") {
            var o = {}, pn = this.paramNames;
            o[pn.start] = this.cursor;
            o[pn.limit] = this.pageSize;
            o['cache'] = 'node';
            if(this.fireEvent('beforechange', this, o) !== false){
              //itemsGrid.getEl().mask('Loading', 'x-mask-loading');
              //this.store.load({params:o,callback:function(){itemsGrid.getEl().unmask();}});
              this.store.load({params:o});
            }
            return;
          }
          Ext.PagingToolbar.prototype.onClick.call(this, which);
        }
      });

      itemsGrid = new Ext.grid.GridPanel({
        id: 'items-grid',
        ds: dataStore,
        columns: columns,
        autoExpandColumn: 'name',
        bbar: bar,
        sm: new Ext.grid.RowSelectionModel({singleSelect:true}),
        
        title: AlfrescoBrowser.Settings['homeText'],
        //title: 'Content Items',
        region: 'center',
        loadMask: true,
        
        listeners: {
          'rowclick': function(grid, dataIndex) {
            var dataRow = dataStore.getAt(dataIndex);
            propsGrid.setSource(dataRow.data);
          },
          'rowdblclick': function(grid, dataIndex) {
            var dataRow = dataStore.getAt(dataIndex);
            AlfrescoBrowser.SendToDrupal(dataRow.data);
        }},
        
        viewConfig: {
          getRowClass: function(record, index) {
            return record.get('nid').length > 0 ? 'row-node-exists' : 'row-node-new';
          }
        },

        tbar: [{
          text: 'Add content',
          tooltip: 'Upload content to this space.',
          iconCls: 'upload',
          handler: function() {
            var space = folderTree.getSelectionModel().getSelectedNode();

            if (!Ext.isEmpty(space)) {
              AlfrescoBrowser.UploadItem(space.id);
            }
          }
        }, '-', {
          text: 'Download',
          tooltip: 'Download selected item.',
          iconCls: 'download',
          handler: function() {
            var items = itemsGrid.getSelectionModel().getSelections();
            if (items.length > 0) {
              var name = items[0].data.name;
              var url = AlfrescoBrowser.Settings['serviceDownloadUrl'] + "/" + name + "?node=" + items[0].data.id + "&mode=attachment";
              if (Ext.isIE) {
                window.location = url;
              }
              else {
                window.open(url);
              }
            }
          }
        }, '-', {
          text: 'View',
          tooltip: 'View selected item.',
          iconCls: 'view',
          handler: function() {
            var items = itemsGrid.getSelectionModel().getSelections();
            if (items.length > 0) {
              var name = items[0].data.name;
              var url = AlfrescoBrowser.Settings['serviceDownloadUrl'] + "/" + name + "?node=" + items[0].data.id;
              AlfrescoBrowser.ViewItem(url, items[0].data.name);
            }
          }
        },'-',{
          text: 'Send to Drupal',
          tooltip: 'Send selected item to Drupal.',
          iconCls: 'drupal',
          handler: function() {
            if (!window.opener) {
              return;
            }
            if (window.opener.$("form#alfresco-import-form").length == 0) {
              var items = itemsGrid.getSelectionModel().getSelections();
              if (items.length > 0) {
                AlfrescoBrowser.SendToDrupal(items[0].data);
              }
            }
            else {
              var node = folderTree.getSelectionModel().getSelectedNode();
              if (!Ext.isEmpty(node)) {
                AlfrescoBrowser.SendToDrupal(node);
              }
            }
          }
        }, '-', {
          id: 'grid-details',
          text: 'Details',
          tooltip: 'View details.',
          iconCls: 'details',
          enableToggle: true,
          toggleHandler: function(item, pressed){
            if (pressed) {
              propsGrid.expand();
            }
            else {
              propsGrid.collapse();
            }
          }
        }],

        tools: [{
          id: 'refresh',
          qtip: 'Clear content and search cache.',
          on: {
            click: function(){
              var o = {start: 0, cache: 'all'};
              //itemsGrid.getEl().mask('Loading', 'x-mask-loading');
              //dataStore.load({params:o,callback:function(){itemsGrid.getEl().unmask();}});
              dataStore.load({params:o});
            }}
        }]
      });
      
      propsGrid = new Ext.grid.PropertyGrid({
        title: 'Properties',
        region: 'south',
        margins: '0 0 0 0',
        cmargins: '5 0 0 0',
        height: 150,
        minSize: 80,
        maxSize: 300,
        collapsible: true,
        collapsed: true,
        split: true,
        //hideCollapseTool: true,
        hideHeaders: true,
        listeners: {
        'validateedit': function(e) {
          e.cancel = true;
        },
        'expand': function(p) {
          Ext.getCmp('grid-details').toggle(true);
        },
        'collapse': function(p) {
          Ext.getCmp('grid-details').toggle(false);
        }}
      });
    },
    initUploadForm: function(){
      // --------------------------------------------
      // -- UPLOAD FORM
      // --------------------------------------------
    }
  };
}();
Ext.onReady(AlfrescoBrowser.App.init, AlfrescoBrowser.App);