#include "parse_svg.h"

void print_photo_location(photo_location_t *photo_location)
{
    printf("-----------------------------------------------------------------------------------\n");
    printf("id     : %d\n",photo_location->id);
    printf("x      : %f\n",photo_location->x);
    printf("y      : %f\n",photo_location->y);
    printf("width  : %f\n",photo_location->width);
    printf("height : %f\n",photo_location->height);
    printf("trans  : %s\n",photo_location->transformation_string);
    printf("-----------------------------------------------------------------------------------\n");
}

int xml_node_contains_child_with_name(xmlNode * a_node, const char *name)
{
    for (xmlNode *child_node = a_node->children; child_node; child_node = child_node->next) {
        if (!strcmp(child_node->name,name)) {
            return 1;
        }
    }
    return 0;
}

int cnt_photos(xmlNode * a_node)
{
    int cnt = 0;
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_contains_child_with_name(cur_node, "rect") && xml_node_contains_child_with_name(cur_node, "text")){
            cnt++;
        }
        cnt += cnt_photos(cur_node->children);
    }
    return cnt;
}



void append_photo_node_to_list(xmlNode * a_node,xmlNode ***photo_nodes, int *index)
{
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_contains_child_with_name(cur_node, "rect") && xml_node_contains_child_with_name(cur_node, "text")){
            (*photo_nodes)[(*index)++] = cur_node;
        }
        append_photo_node_to_list(cur_node->children,photo_nodes,index);
    }
}

void get_photos(xmlNode * a_node, xmlNode ***photo_nodes, int *num_photos)
{
    *num_photos = cnt_photos(a_node);
    *photo_nodes = malloc((*num_photos)*sizeof(xmlNode*)); 
    int i = 0;
    append_photo_node_to_list(a_node,photo_nodes,&i);
}

xmlNode *xml_sibling_node_by_name(xmlNode * a_node, const char *name)
{
    xmlNode *cur_node = NULL;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (!strcmp(cur_node->name,name)) {
            return cur_node;
        }
    }
    return NULL;
}

xmlNode *xml_child_node_by_name(xmlNode * a_node, const char *name)
{
    xmlNode *cur_node = NULL;
    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next) {
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
    xmlNode *svg_element = xml_sibling_node_by_name(root_element,"svg");
    if(svg_element == NULL)return -1;
    xmlNode *photos_node = xml_get_node_where_attribute_has_value(svg_element,"label","photos");
    if(photos_node == NULL)return -1;
   
    xmlNode **photo_nodes;
    int num_photos;
    get_photos(photos_node, &photo_nodes, &num_photos);
    photo_location_t *photo_locations = malloc(num_photos * sizeof(photo_location_t));
    memset(photo_locations,0,num_photos * sizeof(photo_location_t));
    for(int i = 0; i<num_photos ; i++){
        xmlNode *text_node = xml_child_node_by_name(photo_nodes[i],"text");
        xmlNode *rect_node = xml_child_node_by_name(photo_nodes[i],"rect");
        photo_locations[i].id = strtol(text_node->children->children->content,NULL,10);
        photo_locations[i].x = strtod(xmlGetProp(rect_node,"x"),NULL);
        photo_locations[i].y = strtod(xmlGetProp(rect_node,"y"),NULL);
        photo_locations[i].width  = strtod(xmlGetProp(rect_node,"width"),NULL);
        photo_locations[i].height = strtod(xmlGetProp(rect_node,"height"),NULL);
        // for(xmlNode *cur_node = rect_node ; cur_node != svg_element ; cur_node = cur_node->parent){
        //     strcat(photo_locations[i].transformation_string,xmlGetProp(rect_node,"transform"));
        // }
        print_photo_location(&photo_locations[i]);
    }

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