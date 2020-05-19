#include <stdio.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

static void print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }
        print_element_names(cur_node->children);
    }
}

xmlNode *xml_node_by_name(xmlNode * a_node, const char *name)
{
    xmlNode *cur_node = NULL;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (!strcmp(cur_node->name,name)) {
            return cur_node;
        }
    }
    return NULL;
}

xmlNode *xml_get_node_where_attribute_has_value(xmlNode * a_node, const char *attribute_key, const char *attribute_value)
{
    for (xmlNode *child_node = a_node->children; child_node; child_node = child_node->next) {
        if (child_node->type == XML_ELEMENT_NODE) {
            xmlChar *label = xmlGetProp(child_node,attribute_key);
            if(label && !strcmp(label,attribute_value)){
                return child_node;
            }
        }
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    xmlParserCtxtPtr ctxt; /* the parser context */
    xmlDocPtr doc; /* the resulting document tree */
    const char *filename = argv[1];

    /* create a parser context */
    ctxt = xmlNewParserCtxt();
    if (ctxt == NULL) {
        fprintf(stderr, "Failed to allocate parser context\n");
	return 1;
    }
    /* parse the file, activating the DTD validation option */
    doc = xmlReadFile(filename, NULL, 0);
    /* check if parsing suceeded */
    if(doc == NULL)return -1;
    xmlNode *root_element = xmlDocGetRootElement(doc);
    if(root_element == NULL)return -1;
    xmlNode *svg_element = xml_node_by_name(root_element,"svg");
    if(svg_element == NULL)return -1;
    xmlNode *photos_node = xml_get_node_where_attribute_has_value(svg_element,"label","photos");
    if(photos_node == NULL)return -1;
    print_element_names(photos_node);

	/* free up the resulting document */
	xmlFreeDoc(doc);
    return 0;
}

/*
void QDesign::photo_layout_from_svg(QString input_svg)
{
    _photo_layout_list->clear();
    QFile svg_file(input_svg);
    QDomDocument doc;
    if (svg_file.open(QIODevice::ReadOnly)){
        doc.setContent(&svg_file);
        for(int i = 0;i<doc.elementsByTagName("svg").at(0).childNodes().count();i++){
            if(doc.elementsByTagName("svg").at(0).childNodes().at(i).toElement().attributeNode("inkscape:label").value() == "photos"){
                QDomNodeList photos = doc.elementsByTagName("svg").at(0).childNodes().at(i).toElement().elementsByTagName("rect");
                for(int j = 0; j < photos.count();j++){
                    QDomNode node = photos.at(j);
                    int photo_num = node.parentNode().toElement().elementsByTagName("text").at(0).childNodes().at(0).toElement().text().toInt();
                    QRect rect((int)qFloor(node.toElement().attributeNode("x").value().toDouble()),
                               (int)qFloor(node.toElement().attributeNode("y").value().toDouble()),
                               (int)qFloor(node.toElement().attributeNode("width").value().toDouble()),
                               (int)qFloor(node.toElement().attributeNode("height").value().toDouble()));
                    QTransform transformation;
                    while(node.toElement().tagName() != "svg"){
                        transformation *= string_to_transform(node.toElement().attributeNode("transform").value());
                        node = node.parentNode();
                    }
                    _photo_layout_list->push_back(new PhotoLayout(rect,transformation,photo_num));
                }
            }
        }
        svg_file.close();
    }
}
*/