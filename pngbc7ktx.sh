echo hello

# https://github.com/richgel999/bc7enc16
for i in *.png; do
	./bc7enc -l -u1 -p0 -g "${i}" "${i%.png}.dds"
done

# https://github.com/hglm/detex
for i in *.dds; do
	./detex-convert "${i}" "${i%.dds}.ktx"
done
