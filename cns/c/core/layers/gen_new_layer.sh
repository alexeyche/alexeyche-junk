[ -z "$1" ] && echo "Need layer name" && exit 1

if [ -z $2 ]; then
    super_class_name="Poisson"
else
    super_class_name=$2
fi    

base_class_name="Poisson"

name=$1
lower_name=$(echo $name | tr '[:upper:]' '[:lower:]')
upper_name=$(echo $name | tr '[:lower:]' '[:upper:]')
super_lower_name=$(echo $super_class_name | tr '[:upper:]' '[:lower:]')
super_upper_name=$(echo $super_class_name | tr '[:lower:]' '[:upper:]')
base_lower_name=$(echo $base_class_name | tr '[:upper:]' '[:lower:]')
base_upper_name=$(echo $base_class_name | tr '[:lower:]' '[:upper:]')


sed -e "s|{{Name}}|$name|g" -e "s|{{name}}|$lower_name|g" -e "s|{{NAME}}|$upper_name|g" \
    -e "s|{{SuperClassName}}|$super_class_name|g" -e "s|{{superclassname}}|$super_lower_name|g" -e "s|{{SUPERCLASSNAME}}|$super_upper_name|g" \
    -e "s|{{BaseClassName}}|$base_class_name|g" -e "s|{{baseclassname}}|$base_lower_name|g" -e "s|{{BASECLASSNAME}}|$base_upper_name|g" \
    ./new_layer_c.template > ./${lower_name}.c



sed -e "s|{{Name}}|$name|g" -e "s|{{name}}|$lower_name|g" -e "s|{{NAME}}|$upper_name|g"  \
    -e "s|{{SuperClassName}}|$super_class_name|g" -e "s|{{superclassname}}|$super_lower_name|g" -e "s|{{SUPERCLASSNAME}}|$super_upper_name|g" \
    -e "s|{{BaseClassName}}|$base_class_name|g" -e "s|{{baseclassname}}|$base_lower_name|g" -e "s|{{BASECLASSNAME}}|$base_upper_name|g" \
    ./new_layer_h.template > ./${lower_name}.h
