import boto3
import os
import sys

import time

SOURCE_REGION = 'us-west-2'
SOURCE_IMAGE = 'ami-f68f3a96'
IMAGE_NAME = 'universe-gtav-0.0.7'
IMAGE_DESCRIPTION = 'Runs steam version of GTAV as an OpenAI Universe environment'


def main():
    regions = boto3.client('ec2').describe_regions()['Regions']
    new_image_ids = copy_ami(regions)
    make_amis_public(new_image_ids)


def copy_ami(regions):
    image_ids = [(SOURCE_IMAGE, SOURCE_REGION)]
    for region in regions:
        region_name = region['RegionName']
        if region_name == SOURCE_REGION:
            continue
        ec2_dest = boto3.client('ec2', region_name=region_name)
        new_image_id = ec2_dest.copy_image(
            SourceRegion=SOURCE_REGION,
            SourceImageId=SOURCE_IMAGE,
            Name=IMAGE_NAME,
            Description=IMAGE_DESCRIPTION,
        )['ImageId']
        print('New image id', new_image_id, region_name)
        image_ids.append((new_image_id, region_name))
    return image_ids


def make_amis_public(new_image_ids):
    for new_image_id, region_name in new_image_ids:
        made_public = False
        ec2_dest = boto3.client('ec2', region_name=region_name)
        while not made_public:
            try:
                ec2_dest.modify_image_attribute(
                    ImageId=new_image_id,
                    LaunchPermission={'Add': [{'Group': 'all'}]})
                made_public = True
            except Exception as e:
                print('AMI not ready, retrying - details: ', e)
            time.sleep(5)
        print('Image public!', new_image_id, region_name)


if __name__ == '__main__':
    sys.exit(main())


ec2_regions = '''
US East (N. Virginia)	us-east-1	ec2.us-east-1.amazonaws.com	HTTP and HTTPS
US East (Ohio)	us-east-2	ec2.us-east-2.amazonaws.com	HTTP and HTTPS
US West (N. California)	us-west-1	ec2.us-west-1.amazonaws.com	HTTP and HTTPS
US West (Oregon)	us-west-2	ec2.us-west-2.amazonaws.com	HTTP and HTTPS
Canada (Central)	ca-central-1	ec2.ca-central-1.amazonaws.com	HTTP and HTTPS
Asia Pacific (Mumbai)	ap-south-1	ec2.ap-south-1.amazonaws.com	HTTP and HTTPS
Asia Pacific (Seoul)	ap-northeast-2	ec2.ap-northeast-2.amazonaws.com	HTTP and HTTPS
Asia Pacific (Singapore)	ap-southeast-1	ec2.ap-southeast-1.amazonaws.com	HTTP and HTTPS
Asia Pacific (Sydney)	ap-southeast-2	ec2.ap-southeast-2.amazonaws.com	HTTP and HTTPS
Asia Pacific (Tokyo)	ap-northeast-1	ec2.ap-northeast-1.amazonaws.com	HTTP and HTTPS
EU (Frankfurt)	eu-central-1	ec2.eu-central-1.amazonaws.com	HTTP and HTTPS
EU (Ireland)	eu-west-1	ec2.eu-west-1.amazonaws.com	HTTP and HTTPS
EU (London)	eu-west-2	ec2.eu-west-2.amazonaws.com	HTTP and HTTPS
South America (São Paulo)	sa-east-1	ec2.sa-east-1.amazonaws.com	HTTP and HTTPS
'''