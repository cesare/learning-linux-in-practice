Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/xenial64"
  config.vm.provision :shell, path: "scripts/provisioning.sh", privileged: true
  config.vm.synced_folder "./labs", "/home/vagrant/labs", owner: "vagrant", group: "vagrant"

  config.vm.provider "virtualbox" do |vb|
    vb.cpus = 4
    vb.memory = 2048
    vb.customize ["modifyvm", :id, "--ioapic", "on"]
    vb.customize ["guestproperty", "set", :id, "/VirtualBox/GuestAdd/VBoxService/--timesync-set-threshold", 10000]
  end
end
